from hdmlp import hdmlp
from hdmlp.lib.torch import HDMLPImageFolder
from hdmlp.lib.torch import HDMLPDataLoader
from torchvision import datasets, models, transforms
from torch.utils.data import DataLoader
from torchvision.datasets import ImageFolder
import time
from PIL import Image
import io

batch_size = 128
epochs = 10

lib_path = "/Volumes/GoogleDrive/Meine Ablage/Dokumente/1 - Schule/1 - ETHZ/6. Semester/Bachelor Thesis/hdmlp/cpp/hdmlp/cmake-build-debug/libhdmlp.dylib"
config_path = "/Volumes/GoogleDrive/Meine Ablage/Dokumente/1 - Schule/1 - ETHZ/6. Semester/Bachelor Thesis/hdmlp/cpp/hdmlp/data/hdmlp.cfg"
#path = "/tmp/test"
path = "/Volumes/Daten/Daten/Datasets/hymenoptera_data/train" # (245 files)
input_size = 224

job = hdmlp.Job(path,
                batch_size,
                epochs,
                "uniform",
                False,
                None,
                config_path,
                lib_path)

img_transforms = transforms.Compose([
                transforms.RandomResizedCrop(input_size),
                #transforms.RandomHorizontalFlip(),
                transforms.ToTensor(),
                #transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
            ])
hdmlpimagefolder = HDMLPImageFolder(path, job, img_transforms)

imagefolder = ImageFolder(path, None)
dataloader = HDMLPDataLoader(hdmlpimagefolder, batch_size, False, 1, 0)
"""
dataloader = DataLoader(imagefolder, batch_size=batch_size, drop_last=False, num_workers=4)
print("Setup complete")


start = time.time()
for epoch in range(epochs):
    print(epoch)
    for sample, label in dataloader:
        print(label)
        #time.sleep(1)
        #print(label)
        #print(sample)
print(time.time() - start)
"""
epoch = 0
for samples, labels in dataloader:
    epoch += 1
    for i in range(len(labels)):
        label = labels[i]
        sample = samples[i]
        im = transforms.ToPILImage()(sample).convert("RGB")
        im.save("/tmp/test/" + str(label.item()) + "/" + str(i) + str(epoch) + ".jpg")
