import torch
import math
import threading
import queue

class HDMLPThreadedDataLoader(object):

    def __init__(self, dataset, global_batch_size: int, drop_last_batch: bool, num_nodes: int, node_id: int, num_epochs: int):
        self.dataset = dataset
        self.dataset_size = len(self.dataset)
        self.n = num_nodes
        self.global_batch_size = global_batch_size
        self.num_epochs = num_epochs
        # Counts are set to replicate strategy of Sampler.cpp
        self.node_local_batch_size = math.ceil(global_batch_size / num_nodes)
        self.local_batch_size = min(max(global_batch_size - node_id * self.node_local_batch_size, 0), self.node_local_batch_size)
        self.node_id = node_id
        self.drop_last_batch = drop_last_batch
        self.batch_offset = 0
        self.prefetch_batch_offset = 0
        self.prefetch_epoch_offset = 0
        self.queue = queue.Queue()
        self.thread = threading.Thread(target=self.prefetch, daemon=True)
        self.thread.start()

    def __del__(self):
        self.thread.join()

    def __iter__(self):
        return self

    def __next__(self):
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
        return self.queue.get()

    def __len__(self):
        return len(self.dataset)

    def prefetch(self):
        while self.prefetch_epoch_offset < self.num_epochs:
            iter_batch_size = self.local_batch_size
            if self.prefetch_batch_offset > self.dataset_size:
                self.prefetch_batch_offset = 0
                self.prefetch_epoch_offset += 1
            elif self.prefetch_batch_offset + self.global_batch_size > self.dataset_size:
                if self.drop_last_batch:
                    self.prefetch_batch_offset = 0
                    self.prefetch_epoch_offset += 1
                else:
                    # Replicate Sampler.cpp, i.e. nodes iterate still node_local_batch_size in the last batch, unless their offset is higher than the file size
                    iter_batch_size = min(max((self.dataset_size - self.prefetch_batch_offset) - self.node_local_batch_size * self.node_id, 0), self.node_local_batch_size)
                    if iter_batch_size == 0:
                        self.prefetch_batch_offset = 0
                        self.prefetch_epoch_offset += 1

            labels = []
            samples = []
            for i in range(iter_batch_size):
                sample, label = self.dataset[0]
                labels.append(torch.as_tensor(label))
                samples.append(sample)
            self.prefetch_batch_offset += self.global_batch_size
            self.queue.put((torch.stack(samples, 0), torch.stack(labels, 0)))