from lib.torch.hdmlpimagenet import HDMLPImageNet
import hdmlp

batch_size = 128
epochs = 1
path = "/Volumes/Data/Bachelor Thesis/datasets/imagenet"
split = "train"
distr_scheme = "uniform"
transform = None
target_transform = None

job = hdmlp.Job(path + "/" + split,
                batch_size,
                epochs,
                distr_scheme,
                True,
                None)

imagenet_data = HDMLPImageNet(path, job, transform, target_transform, split)
for sample in imagenet_data:
    print(sample)