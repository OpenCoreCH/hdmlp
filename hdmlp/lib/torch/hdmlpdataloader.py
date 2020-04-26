import torch
import math

class HDMLPDataLoader(object):

    def __init__(self, dataset):
        self.dataset = dataset
        self.dataset_size = len(self.dataset)
        job = self.dataset.get_job()
        self.n = job.get_no_nodes()
        self.node_id = job.get_node_id()
        self.global_batch_size = job.get_batch_size()
        self.num_epochs = job.get_num_epochs()
        # Counts are set to replicate strategy of Sampler.cpp
        self.node_local_batch_size = math.ceil(self.global_batch_size / self.n)
        self.local_batch_size = min(max(self.global_batch_size - self.node_id * self.node_local_batch_size, 0), self.node_local_batch_size)
        self.drop_last_batch = job.get_drop_last_batch()
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
                iter_batch_size = min(max((self.dataset_size - self.batch_offset) - self.node_local_batch_size * self.node_id, 0), self.node_local_batch_size)
                if iter_batch_size == 0:
                    self.batch_offset = 0
                    raise StopIteration

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
