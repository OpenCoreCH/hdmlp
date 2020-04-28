import torch
import torch.nn as nn
import torch.optim as optim
import torch.distributed as dist
import torch.utils.data
import horovod.torch as hvd
from torchvision import datasets, models, transforms
import time
import os
import copy
import hdmlp
import hdmlp.lib.torch
import argparse

"""
Distributed PyTorch Classification benchmark, based on:
 - https://pytorch.org/tutorials/beginner/finetuning_torchvision_models_tutorial.html
 - https://github.com/horovod/horovod/blob/master/docs/pytorch.rst
"""
# Parse input arguments
parser = argparse.ArgumentParser(description='Distributed PyTorch Benchmark Script')
parser.add_argument('--backend', choices={"hdmlp", "torchvision"}, default="hdmlp")
parser.add_argument('--epochs', type=int, default=10)
parser.add_argument('--data-dir', type=str)
parser.add_argument('--batch-size', type=int, default=32)
parser.add_argument('--lib-path', type=str, default=None)
parser.add_argument('--config-path', type=str, default=None)
parser.add_argument('--drop-last-batch', type=bool, default=False)
parser.add_argument('--seed', type=int, default=None)
parser.add_argument('--model-name', choices={"resnet", "alexnet", "vgg", "squeezenet", "densenet", "inception"})
parser.add_argument('--num-classes', type=int, default=2)
parser.add_argument('--torch-num-workers', type=int, default=4)
parser.add_argument('--dataset', choices={"folder", "imagenet"}, default="folder")
parser.add_argument('--imagenet-devkit-root', type=str, default=None)
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

# PyTorch parameters
torch_num_workers = args.torch_num_workers
# HDMLP parameters
lib_path = args.lib_path
config_path = args.config_path
drop_last_batch = args.drop_last_batch
seed = args.seed

def train_model(model, dataloaders, criterion, optimizer, num_epochs=25, is_inception=False, node_id = 0):
    since = time.time()
    load_time = 0

    val_acc_history = []

    best_model_wts = copy.deepcopy(model.state_dict())
    best_acc = 0.0

    for epoch in range(num_epochs):
        if node_id == 0:
            print('Epoch {}/{}'.format(epoch + 1, num_epochs))
            print('-' * 10)

        # Each epoch has a training and validation phase
        for phase in ['train', 'val']:
            if phase == 'train':
                model.train()  # Set model to training mode
            else:
                model.eval()  # Set model to evaluate mode

            running_loss = 0.0
            running_corrects = 0

            # Iterate over data.
            iter_items = 0
            bef_time = time.time()
            for inputs, labels in dataloaders[phase]:
                load_time += time.time() - bef_time
                inputs = inputs.to(device)
                labels = labels.to(device)
                iter_items += len(labels)

                # zero the parameter gradients
                optimizer.zero_grad()

                # forward
                # track history if only in train
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

                # statistics
                running_loss += loss.item() * inputs.size(0)
                running_corrects += torch.sum(preds == labels.data)
                bef_time = time.time()

            epoch_loss = running_loss / iter_items
            epoch_acc = running_corrects.double() / iter_items

            if node_id == 0:
                print('{} Loss: {:.4f} Acc: {:.4f}'.format(phase, epoch_loss, epoch_acc))

            # deep copy the model
            if phase == 'val' and epoch_acc > best_acc:
                best_acc = epoch_acc
                best_model_wts = copy.deepcopy(model.state_dict())
            if phase == 'val':
                val_acc_history.append(epoch_acc)

        print()

    time_elapsed = time.time() - since
    load_time_avg = hvd.allreduce(torch.tensor(load_time))
    if node_id == 0:
        print('Training complete in {:.0f}m {:.0f}s'.format(time_elapsed // 60, time_elapsed % 60))
        print('Best val Acc: {:4f}'.format(best_acc))
        print('Load time: {}'.format(load_time_avg.item()))

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

    if model_name == "resnet":
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

if __name__ == "__main__":
    # Initialize the model for this run
    model_ft, input_size = initialize_model(model_name, num_classes, feature_extract, use_pretrained=False)
    # Data augmentation and normalization for training
    # Just normalization for validation
    data_transforms = {
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

    image_datasets = {}
    dataloaders_dict = {}

    # Create training and validation datasets

    if backend == "hdmlp":
        hdmlp_jobs = {x: hdmlp.Job(os.path.join(data_dir, x), batch_size, num_epochs, 'uniform', drop_last_batch, seed, config_path, lib_path) for x in ['train', 'val']}
        if dataset == "folder":
            image_datasets = {x: hdmlp.lib.torch.HDMLPImageFolder(os.path.join(data_dir, x), hdmlp_jobs[x], data_transforms[x]) for x in ['train', 'val']}
        elif dataset == "imagenet":
            image_datasets = {x: hdmlp.lib.torch.HDMLPImageNet(data_dir, hdmlp_jobs[x], data_transforms[x], split=x, devkit_root=imagenet_devkit_root) for x in
                              ['train', 'val']}
        dataloaders_dict = {x: hdmlp.lib.torch.HDMLPThreadedDataLoader(image_datasets[x]) for x in ['train', 'val']}
        hvd.init()
    elif backend == "torchvision":
        hvd.init()
        if dataset == "folder":
            image_datasets = {x: datasets.ImageFolder(os.path.join(data_dir, x), data_transforms[x]) for x in ['train', 'val']}
        elif dataset == "imagenet":
            image_datasets = {x: datasets.ImageNet(os.path.join(data_dir, x), split=x, transform=data_transforms[x]) for x in ['train', 'val']}
        image_samplers = {x: torch.utils.data.distributed.DistributedSampler(image_datasets[x], num_replicas=hvd.size(), rank=hvd.rank()) for x in ['train', 'val']}
        dataloaders_dict = {x: torch.utils.data.DataLoader(image_datasets[x], batch_size=int(batch_size / hvd.size()), num_workers=torch_num_workers, sampler=image_samplers[x]) for x in ['train', 'val']}

    num_nodes = hvd.size()
    node_id = hvd.rank()

    torch_device = "cuda:0" if torch.cuda.is_available() else "cpu"
    device = torch.device(torch_device)
    model_ft = model_ft.to(device)

    if node_id == 0:
        print("Backend: {}".format(backend))
        print("Dataset: {}".format(dataset))
        print("Dataset path: {}".format(data_dir))
        print("Device: {}".format(torch_device))

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
    optimizer_ft = hvd.DistributedOptimizer(optimizer_ft, named_parameters=model_ft.named_parameters())
    hvd.broadcast_parameters(model_ft.state_dict(), root_rank=0)
    # Setup the loss fxn
    criterion = nn.CrossEntropyLoss()
    # Train and evaluate
    model_ft, hist = train_model(model_ft, dataloaders_dict, criterion, optimizer_ft, num_epochs=num_epochs,
                                 is_inception=(model_name == "inception"), node_id=node_id)