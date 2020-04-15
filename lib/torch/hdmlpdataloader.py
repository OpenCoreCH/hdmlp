import torch
import math

class HDMLPDataLoader(object):

    def __init__(self, dataset, global_batch_size: int, drop_last_batch: bool, num_nodes: int, node_id: int):
        self.dataset = dataset
        self.dataset_size = len(self.dataset)
        self.n = num_nodes
        self.global_batch_size = global_batch_size
        # Counts are set to replicate strategy of Sampler.cpp
        self.node_local_batch_size = math.ceil(global_batch_size / num_nodes)
        self.local_batch_size = min(max(global_batch_size - node_id * self.node_local_batch_size, 0), self.node_local_batch_size)
        self.node_id = node_id
        self.drop_last_batch = drop_last_batch
        self.batch_offset = 0

    def __iter__(self):
        return self

    def __next__(self):
        iter_batch_size = self.local_batch_size
        if self.batch_offset > self.dataset_size:
            self.batch_offset = 0
            raise StopIteration
        elif self.batch_offset + self.global_batch_size > self.dataset_size:
            if self.drop_last_batch:
                self.batch_offset = 0
                raise StopIteration
            else:
                # Replicate Sampler.cpp, i.e. nodes iterate still node_local_batch_size in the last batch, unless their offset is higher than the file size
                iter_batch_size = max((self.dataset_size - self.batch_offset) - self.node_local_batch_size * self.node_id, 0)

        labels = []
        samples = []
        for i in range(iter_batch_size):
            sample, label = self.dataset[0]
            labels.append(torch.as_tensor(label))
            samples.append(sample)
        self.batch_offset += self.global_batch_size
        return torch.stack(samples, 0), torch.stack(labels, 0)

    def __len__(self):
        return len(self.dataset)