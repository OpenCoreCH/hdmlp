import torch
import torch.nn as nn
import torch.optim as optim
import torch.distributed as dist
import torch.utils.data
from torchvision import datasets, models, transforms
import time
import os
import copy
import hdmlp
import hdmlp.lib.torch
import argparse
import pickle
from datetime import datetime
import random

"""
Distributed PyTorch Classification benchmark, based on:
 - https://pytorch.org/tutorials/beginner/finetuning_torchvision_models_tutorial.html
 - https://github.com/ShigekiKarita/pytorch-distributed-slurm-example/blob/master/main_distributed.py
"""
# Parse input arguments
parser = argparse.ArgumentParser(description='Distributed PyTorch Benchmark Script')
parser.add_argument('--backend', choices={"hdmlp", "torchvision"}, default="hdmlp")
parser.add_argument('--epochs', type=int, default=10)
parser.add_argument('--data-dir', type=str)
parser.add_argument('--batch-size', type=int, default=32)
parser.add_argument('--lib-path', type=str, default=None)
parser.add_argument('--config-path', type=str, default=None)
parser.add_argument('--val-config-path', type=str, default=None)
parser.add_argument('--drop-last-batch', type=bool, default=False)
parser.add_argument('--seed', type=int, default=None)
parser.add_argument('--model-name', choices={"resnet50", "resnet18", "alexnet", "vgg", "squeezenet", "densenet", "inception"})
parser.add_argument('--num-classes', type=int, default=2)
parser.add_argument('--torch-num-workers', type=int, default=4)
parser.add_argument('--dataset', choices={"folder", "imagenet"}, default="folder")
parser.add_argument('--imagenet-devkit-root', type=str, default=None)
parser.add_argument('--init-shared-file', type=str)
parser.add_argument('--stat-location', type=str, default=None, help="Folder to output run statistics")
parser.add_argument('--do-val', type=bool, default=False, help="Run validation after training")
args = parser.parse_args()

# --- Benchmark parameters ---
# Top level data directory. Here we assume the format of the directory conforms to the ImageFolder structure
data_dir = args.data_dir

# Models to choose from [resnet, alexnet, vgg, squeezenet, densenet, inception]
model_name = args.model_name
# Number of classes in the dataset
num_classes = args.num_classes
# Batch size for training (change depending on how much memory you have)
batch_size = args.batch_size
# Number of epochs to train for
num_epochs = args.epochs
# Flag for feature extracting. When False, we finetune the whole model, when True we only update the reshaped layer params
feature_extract = False
# Which file loading framework to use
backend = args.backend
dataset = args.dataset
imagenet_devkit_root = args.imagenet_devkit_root
num_nodes = 1
if "SLURM_JOB_NUM_NODES" in os.environ:
    num_nodes = int(os.environ['SLURM_JOB_NUM_NODES'])
elif "OMPI_COMM_WORLD_SIZE" in os.environ:
    num_nodes = int(os.environ['OMPI_COMM_WORLD_SIZE'])
distributed = num_nodes > 1
node_id = 0
if "SLURM_PROCID" in os.environ:
    node_id = int(os.environ['SLURM_PROCID'])
elif "OMPI_COMM_WORLD_RANK" in os.environ:
    node_id = int(os.environ['OMPI_COMM_WORLD_RANK'])
torch_node_id = node_id
hdmlp_node_id = 0
init_shared_file = args.init_shared_file
jobid = random.randint(0, 100000000)
if "SLURM_JOBID" in os.environ:
    jobid = os.environ["SLURM_JOBID"]
profiling = False

# PyTorch parameters
torch_num_workers = args.torch_num_workers
# HDMLP parameters
lib_path = args.lib_path
config_path = args.config_path
val_config_path = args.val_config_path
if val_config_path is None:
    val_config_path = config_path
drop_last_batch = args.drop_last_batch
seed = args.seed
if args.stat_location is not None:
    # Collect Profiling information
    profiling = True
    compute_times = []
    os.environ["HDMLPPROFILING"] = "1"

if node_id == 0:
    print("Launched at {}".format(datetime.now().strftime("%H:%M:%S")))

def train_model(model, dataloaders, criterion, optimizer, num_epochs=25, is_inception=False, node_id = 0, hdmlp_val_job = None):
    if node_id == 0:
        print("Starting training at {}".format(datetime.now().strftime("%H:%M:%S")))
    since = time.time()
    load_time = 0
    train_time = 0

    val_acc_history = []

    best_model_wts = copy.deepcopy(model.state_dict())
    best_acc = 0.0

    for epoch in range(num_epochs):
        if node_id == 0:
            print('Epoch {}/{}'.format(epoch + 1, num_epochs))
            print('-' * 10)

        phases = ['train']
        if epoch == num_epochs - 1 and args.do_val:
            phases = ['train', 'val']
        for phase in phases:
            if phase == 'train':
                model.train()  # Set model to training mode
            else:
                if backend == "hdmlp":
                    #dataloaders['train'].dataset.__del__()
                    dataset = hdmlp.lib.torch.HDMLPImageFolder(os.path.join(data_dir, 'val'), hdmlp_val_job)
                    dataloaders['val'] = hdmlp.lib.torch.HDMLPDataLoader(dataset)

                model.eval()  # Set model to evaluate mode

            running_loss = 0.0
            running_corrects = 0

            # Iterate over data.
            iter_items = 0
            iteration = 0
            bef_time = time.time()
            for inputs, labels in dataloaders[phase]:
                iteration += 1
                if phase == "train":
                    load_time += time.time() - bef_time
                inputs = inputs.to(device)
                labels = labels.to(device)
                iter_items += len(labels)

                # zero the parameter gradients
                optimizer.zero_grad()

                # forward
                # track history if only in train
                if profiling and phase == "train":
                    bef_train = time.time()
                with torch.set_grad_enabled(phase == 'train'):
                    # Get model outputs and calculate loss
                    # Special case for inception because in training it has an auxiliary output. In train
                    #   mode we calculate the loss by summing the final output and the auxiliary output
                    #   but in testing we only consider the final output.
                    if is_inception and phase == 'train':
                        # From https://discuss.pytorch.org/t/how-to-optimize-inception-model-with-auxiliary-classifiers/7958
                        outputs, aux_outputs = model(inputs)
                        loss1 = criterion(outputs, labels)
                        loss2 = criterion(aux_outputs, labels)
                        loss = loss1 + 0.4 * loss2
                    else:
                        outputs = model(inputs)
                        loss = criterion(outputs, labels)

                    _, preds = torch.max(outputs, 1)

                    # backward + optimize only if in training phase
                    if phase == 'train':
                        loss.backward()
                        optimizer.step()
                    if iteration % 100 == 0:
                        print('Iteration {}, Load time: {} ({} / {})'.format(iteration, load_time, torch_node_id, hdmlp_node_id))

                # statistics
                if profiling and phase == "train":
                    compute_times.append(time.time() - bef_train)
                running_loss += loss.item() * inputs.size(0)
                running_corrects += torch.sum(preds == labels.data)
                bef_time = time.time()

            epoch_loss = running_loss / iter_items
            epoch_acc = running_corrects.double() / iter_items

            if node_id == 0:
                print('{} Loss: {:.4f} Acc: {:.4f}'.format(phase, epoch_loss, epoch_acc))
                print('Load time: {}'.format(load_time))

            # deep copy the model
            if phase == 'val' and epoch_acc > best_acc:
                best_acc = epoch_acc
                best_model_wts = copy.deepcopy(model.state_dict())
            if phase == 'val':
                val_acc_history.append(epoch_acc)
            if phase == "train" and epoch == num_epochs - 1:
                overall_train_time = time.time() - since

        print()

    if args.stat_location is not None:
        f = open("{}/{}_{}".format(args.stat_location, jobid, node_id), "wb")
        if backend == "hdmlp":
            pickle.dump({
                'backend': 'hdmlp',
                'hdmlp_metrics': dataloaders['train'].job.get_metrics(),
                'compute_times': compute_times,
                'train_time': overall_train_time
            }, f)
        f.close()
    
    time_elapsed = time.time() - since
    if node_id == 0:
        print('Training complete in {:.0f}m {:.0f}s'.format(time_elapsed // 60, time_elapsed % 60))
        print('Best val Acc: {:4f}'.format(best_acc))
        print('Load time: {}'.format(load_time))
        print('Training time: {}'.format(overall_train_time))

    # load best model weights
    model.load_state_dict(best_model_wts)
    return model, val_acc_history


def set_parameter_requires_grad(model, feature_extracting):
    if feature_extracting:
        for param in model.parameters():
            param.requires_grad = False


def initialize_model(model_name, num_classes, feature_extract, use_pretrained=True):
    # Initialize these variables which will be set in this if statement. Each of these
    #   variables is model specific.
    model_ft = None
    input_size = 0

    if model_name == "resnet50":
        """ Resnet50
        """
        model_ft = models.resnet50(pretrained=use_pretrained)
        set_parameter_requires_grad(model_ft, feature_extract)
        num_ftrs = model_ft.fc.in_features
        model_ft.fc = nn.Linear(num_ftrs, num_classes)
        input_size = 224

    elif model_name == "resnet18":
        """ Resnet18
        """
        model_ft = models.resnet18(pretrained=use_pretrained)
        set_parameter_requires_grad(model_ft, feature_extract)
        num_ftrs = model_ft.fc.in_features
        model_ft.fc = nn.Linear(num_ftrs, num_classes)
        input_size = 224

    elif model_name == "alexnet":
        """ Alexnet
        """
        model_ft = models.alexnet(pretrained=use_pretrained)
        set_parameter_requires_grad(model_ft, feature_extract)
        num_ftrs = model_ft.classifier[6].in_features
        model_ft.classifier[6] = nn.Linear(num_ftrs, num_classes)
        input_size = 224

    elif model_name == "vgg":
        """ VGG11_bn
        """
        model_ft = models.vgg11_bn(pretrained=use_pretrained)
        set_parameter_requires_grad(model_ft, feature_extract)
        num_ftrs = model_ft.classifier[6].in_features
        model_ft.classifier[6] = nn.Linear(num_ftrs, num_classes)
        input_size = 224

    elif model_name == "squeezenet":
        """ Squeezenet
        """
        model_ft = models.squeezenet1_0(pretrained=use_pretrained)
        set_parameter_requires_grad(model_ft, feature_extract)
        model_ft.classifier[1] = nn.Conv2d(512, num_classes, kernel_size=(1, 1), stride=(1, 1))
        model_ft.num_classes = num_classes
        input_size = 224

    elif model_name == "densenet":
        """ Densenet
        """
        model_ft = models.densenet121(pretrained=use_pretrained)
        set_parameter_requires_grad(model_ft, feature_extract)
        num_ftrs = model_ft.classifier.in_features
        model_ft.classifier = nn.Linear(num_ftrs, num_classes)
        input_size = 224

    elif model_name == "inception":
        """ Inception v3
        Be careful, expects (299,299) sized images and has auxiliary output
        """
        model_ft = models.inception_v3(pretrained=use_pretrained)
        set_parameter_requires_grad(model_ft, feature_extract)
        # Handle the auxilary net
        num_ftrs = model_ft.AuxLogits.fc.in_features
        model_ft.AuxLogits.fc = nn.Linear(num_ftrs, num_classes)
        # Handle the primary net
        num_ftrs = model_ft.fc.in_features
        model_ft.fc = nn.Linear(num_ftrs, num_classes)
        input_size = 299

    else:
        print("Invalid model name, exiting...")
        exit()

    return model_ft, input_size

def find_free_port():
    import socket
    s = socket.socket()
    s.bind(('', 0))            # Bind to a free port provided by the host.
    return s.getsockname()[1]  # Return the port number assigned.


if __name__ == "__main__":
    if num_nodes > 1:
        file_path = '{}{}'.format(init_shared_file, jobid)
        dist_url = None
        if node_id == 0:
            import socket
            ip = socket.gethostbyname(socket.gethostname())
            port = find_free_port()
            dist_url = "tcp://{}:{}".format(ip, port)
            with open(file_path, "w") as f:
                f.write(dist_url)
        else:
            import os
            import time
            while not os.path.exists(file_path):
                time.sleep(1)
            time.sleep(1)
            with open(file_path, "r") as f:
                dist_url = f.read()
        dist.init_process_group('nccl', init_method=dist_url, rank=node_id, world_size=num_nodes)

    # Initialize the model for this run
    model_ft, input_size = initialize_model(model_name, num_classes, feature_extract, use_pretrained=False)

    # Data augmentation and normalization for training
    # Just normalization for validation
    data_transforms_torchvision = {
        'train': transforms.Compose([
            transforms.RandomResizedCrop(input_size),
            transforms.RandomHorizontalFlip(),
            transforms.ToTensor(),
            transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
        ]),
        'val': transforms.Compose([
            transforms.Resize(input_size),
            transforms.CenterCrop(input_size),
            transforms.ToTensor(),
            transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
        ]),
    }
    data_transforms_hdmlp = {
        'train': [
            hdmlp.lib.transforms.ImgDecode(),
            hdmlp.lib.transforms.RandomResizedCrop(input_size),
            hdmlp.lib.transforms.RandomHorizontalFlip(),
            hdmlp.lib.transforms.ToTensor(),
            hdmlp.lib.transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
        ],
        'val': [
            hdmlp.lib.transforms.ImgDecode(),
            hdmlp.lib.transforms.Resize(input_size, input_size),
            hdmlp.lib.transforms.ToTensor(),
            hdmlp.lib.transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
        ],
    }

    image_datasets = {}
    dataloaders_dict = {}

    # Create training and validation datasets

    hdmlp_val_job = None
    if backend == "hdmlp":
        hdmlp_job = hdmlp.Job(os.path.join(data_dir, 'train'), batch_size * num_nodes, num_epochs, 'uniform', drop_last_batch, data_transforms_hdmlp['train'], seed, config_path, lib_path)
        hdmlp_val_job = hdmlp.Job(os.path.join(data_dir, 'val'), batch_size * num_nodes, 1, 'uniform', drop_last_batch, data_transforms_hdmlp['val'], seed, val_config_path, lib_path)
        if dataset == "folder":
            image_datasets = {'train': hdmlp.lib.torch.HDMLPImageFolder(os.path.join(data_dir, 'train'), hdmlp_job),
                              'val': datasets.ImageFolder(os.path.join(data_dir, 'val'), data_transforms_torchvision['val'])}
        elif dataset == "imagenet":
            image_datasets = {'train': hdmlp.lib.torch.HDMLPImageNet(data_dir, hdmlp_job, data_transforms['train'], split='train', devkit_root=imagenet_devkit_root),
                              'val': datasets.ImageNet(os.path.join(data_dir, 'val'), split='val', transform=data_transforms_torchvision['val'])}
        hdmlp_node_id = image_datasets['train'].get_job().get_node_id()
        torch_sampler = torch.utils.data.distributed.DistributedSampler(image_datasets['val'], num_replicas=num_nodes, rank=node_id)
        dataloaders_dict = {'train': hdmlp.lib.torch.HDMLPDataLoader(image_datasets['train']),
                            'val': torch.utils.data.DataLoader(image_datasets['val'], batch_size=batch_size, num_workers=0, sampler=torch_sampler)}
    elif backend == "torchvision":
        if dataset == "folder":
            image_datasets = {x: datasets.ImageFolder(os.path.join(data_dir, x), data_transforms_torchvision[x]) for x in ['train', 'val']}
        elif dataset == "imagenet":
            image_datasets = {x: datasets.ImageNet(os.path.join(data_dir, x), split=x, transform=data_transforms_torchvision[x]) for x in ['train', 'val']}
        image_samplers = {x: torch.utils.data.distributed.DistributedSampler(image_datasets[x], num_replicas=num_nodes, rank=node_id) for x in ['train', 'val']}
        dataloaders_dict = {'train': torch.utils.data.DataLoader(image_datasets['train'], batch_size=batch_size, num_workers=torch_num_workers, sampler=image_samplers['train']),
                            'val': torch.utils.data.DataLoader(image_datasets['val'], batch_size=batch_size, num_workers=0, sampler=image_samplers['val'])}

    torch_device = "cuda:0" if torch.cuda.is_available() else "cpu"
    device = torch.device(torch_device)
    if distributed:
        torch.cuda.set_device(0)
        model_ft.cuda(0)
        model_ft = torch.nn.parallel.DistributedDataParallel(model_ft, device_ids=[0])
    else:
        model_ft = model_ft.to(device)

    if node_id == 0:
        print("Backend: {}".format(backend))
        print("Dataset: {}".format(dataset))
        print("Dataset path: {}".format(data_dir))
        print("Device: {}".format(torch_device))
        print("Nodes: {}".format(num_nodes))

    # Gather the parameters to be optimized/updated in this run. If we are
    #  finetuning we will be updating all parameters. However, if we are
    #  doing feature extract method, we will only update the parameters
    #  that we have just initialized, i.e. the parameters with requires_grad
    #  is True.
    params_to_update = model_ft.parameters()
    if feature_extract:
        params_to_update = []
        for name, param in model_ft.named_parameters():
            if param.requires_grad:
                params_to_update.append(param)

    # Observe that all parameters are being optimized
    optimizer_ft = optim.SGD(params_to_update, lr=num_nodes * 0.001, momentum=0.9)
    # Setup the loss fxn
    criterion = nn.CrossEntropyLoss()
    # Train and evaluate
    model_ft, hist = train_model(model_ft, dataloaders_dict, criterion, optimizer_ft, num_epochs=num_epochs,
                                 is_inception=(model_name == "inception"), node_id=node_id, hdmlp_val_job = hdmlp_val_job)