#include <iostream>
#include "../include/MemoryPrefetcher.h"
#include "../include/MetadataStore.h"

MemoryPrefetcher::MemoryPrefetcher(std::map<std::string, std::string> &backend_options,
                                   std::vector<int>::const_iterator prefetch_start,
                                   std::vector<int>::const_iterator prefetch_end,
                                   unsigned long long int capacity, StorageBackend* backend,
                                   MetadataStore* metadata_store,
                                   int storage_level, bool alloc_buffer) {
    if (alloc_buffer) {
        buffer = new char[capacity];
        buffer_allocated = true;
    }
    this->prefetch_start = prefetch_start;
    this->prefetch_end = prefetch_end;
    this->backend = backend;
    this->metadata_store = metadata_store;
    this->storage_level = storage_level;
    this->capacity = capacity;
    num_elems = std::distance(prefetch_start, prefetch_end);
    file_ends = new unsigned long long int[num_elems];
}

MemoryPrefetcher::~MemoryPrefetcher() {
    if (buffer_allocated) {
        delete[] buffer;
    }
    delete[] file_ends;
}

void MemoryPrefetcher::prefetch() {
    int local_offset = 0;
    while (true) {
        std::unique_lock<std::mutex> crit_section_lock(prefetcher_mutex);
        local_offset = prefetch_offset;
        if (local_offset >= num_elems) {
            break;
        }
        prefetch_offset++;
        auto ptr = prefetch_start + local_offset;
        int file_id = *ptr;
        unsigned long long prev_end = 0;
        if (local_offset != 0) {
            prev_end = file_ends[local_offset - 1];
        }
        unsigned long file_size = backend->get_file_size(file_id);
        file_ends[local_offset] = prev_end + file_size;
        buffer_offsets[file_id] = local_offset;
        crit_section_lock.unlock();

        backend->fetch(file_id, buffer + prev_end);
        metadata_store->insert_cached_file(storage_level, file_id);
    }
}

void MemoryPrefetcher::fetch(int file_id, char *dst) {
    int offset = buffer_offsets[file_id];
    unsigned long long start;
    unsigned long long end = file_ends[offset];
    if (offset == 0) {
        start = 0;
    } else {
        start = file_ends[offset - 1];
    }
    memcpy(dst, buffer + start, end - start);
}
