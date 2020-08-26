import torch
import math

class HDMLPDataLoader(object):

    def __init__(self, dataset):
        self.dataset = dataset
        self.dataset_size = len(self.dataset)
        self.job = self.dataset.get_job()
        self.n = self.job.get_no_nodes()
        self.node_id = self.job.get_node_id()
        self.global_batch_size = self.job.get_batch_size()
        self.num_epochs = self.job.get_num_epochs()
        # Counts are set to replicate strategy of Sampler.cpp
        self.node_local_batch_size = math.ceil(self.global_batch_size / self.n)
        self.local_batch_size = min(max(self.global_batch_size - self.node_id * self.node_local_batch_size, 0), self.node_local_batch_size)
        self.drop_last_batch = self.job.get_drop_last_batch()
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


        self.batch_offset += self.global_batch_size

        samples, labels = self.dataset[0:iter_batch_size]
        if isinstance(labels, list):
            # Batch mode
            labels = [torch.as_tensor(label) for label in labels]
            return samples, torch.stack(labels, 0)
        else:
            sample_list = [samples]
            label_list = [torch.as_tensor(labels)]
            for i in range(iter_batch_size - 1):
                sample, label = self.dataset[0]
                label_list.append(torch.as_tensor(label))
                sample_list.append(sample)
            return torch.stack(sample_list, 0), torch.stack(label_list, 0)

    def __len__(self):
        return len(self.dataset)
