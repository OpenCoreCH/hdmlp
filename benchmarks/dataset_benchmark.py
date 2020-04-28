import hdmlp
import hdmlp.lib.torch
import math
import time
import random
import argparse
from torchvision.datasets import ImageFolder

parser = argparse.ArgumentParser(description='Dataset Benchmark Script')
parser.add_argument('--backend', choices={"hdmlp", "torchvision"}, default="hdmlp")
parser.add_argument('--epochs', type=int, default=10)
parser.add_argument('--data-dir', type=str)
parser.add_argument('--batch-size', type=int, default=32)
parser.add_argument('--lib-path', type=str, default=None)
parser.add_argument('--config-path', type=str, default=None)
parser.add_argument('--drop-last-batch', type=bool, default=False)
parser.add_argument('--seed', type=int, default=None)
args = parser.parse_args()

backend = args.backend

global_batch_size = args.batch_size
epochs = args.epochs
drop_last_batch = args.drop_last_batch
seed = args.seed

# HDMLP parameters
lib_path = args.lib_path
config_path = args.config_path

# Top level data directory. Here we assume the format of the directory conforms to the ImageFolder structure
path = args.data_dir

if backend == "hdmlp":
    job = hdmlp.Job(path,
                    global_batch_size,
                    epochs,
                    "uniform",
                    drop_last_batch,
                    seed,
                    config_path,
                    lib_path)
    imagefolder = hdmlp.lib.torch.HDMLPImageFolder(path, job)
    node_id = job.get_node_id()
    n = job.get_no_nodes()
elif backend == "torchvision":
    from mpi4py import MPI
    comm = MPI.COMM_WORLD
    n = comm.Get_size()
    node_id = comm.Get_rank()
    imagefolder = ImageFolder(path)


node_local_batch_size = math.ceil(global_batch_size / n)
local_batch_size = min(max(global_batch_size - node_id * node_local_batch_size, 0), node_local_batch_size)
batch_offset = 0
dataset_size = len(imagefolder)

epoch = 0
start = time.time()
# Simulate reads of DataLoader, ensures same number of reads for HDMLP / PyTorch
while epoch < epochs:
    iter_batch_size = local_batch_size
    if batch_offset > dataset_size:
        batch_offset = 0
        epoch += 1
        continue
    elif batch_offset + global_batch_size > dataset_size:
        if drop_last_batch:
            batch_offset = 0
            epoch += 1
            continue
        else:
            iter_batch_size = min(max((dataset_size - batch_offset) - node_local_batch_size * node_id, 0), node_local_batch_size)
            if iter_batch_size == 0:
                batch_offset = 0
                epoch += 1
                continue

    labels = []
    samples = []
    for i in range(iter_batch_size):
        index = 0
        if backend == "torchvision":
            index = random.randrange(dataset_size)
        sample, label = imagefolder[index]
    batch_offset += global_batch_size

print("Backend: {}".format(backend))
print("Number of nodes: {}".format(n))
print("Path: {}".format(path))
print("Total Fetch time: ")
print(time.time() - start)