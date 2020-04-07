import torch

class HDMLPDataLoader(object):

    def __init__(self, dataset, batch_size: int, drop_last_batch: bool):
        self.dataset = dataset
        self.dataset_size = len(self.dataset)
        self.batch_size = batch_size
        self.drop_last_batch = drop_last_batch
        self.batch_offset = 0

    def __iter__(self):
        return self

    def __next__(self):
        iter_batch_size = self.batch_size
        if self.batch_offset > self.dataset_size:
            self.batch_offset = 0
            raise StopIteration
        elif self.batch_offset + self.batch_size > self.dataset_size:
            if self.drop_last_batch:
                self.batch_offset = 0
                raise StopIteration
            else:
                iter_batch_size = self.dataset_size - self.batch_offset

        labels = []
        samples = []
        for i in range(iter_batch_size):
            sample, label = self.dataset[0]
            labels.append(torch.as_tensor(label))
            samples.append(sample)
        self.batch_offset += self.batch_size
        return torch.stack(samples, 0), torch.stack(labels, 0)

    def __len__(self):
        return len(self.dataset)