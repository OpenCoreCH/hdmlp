#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include "../include/FileSystemPrefetcher.h"
#include "../include/MetadataStore.h"

FileSystemPrefetcher::FileSystemPrefetcher(std::map<std::string, std::string> &backend_options,
                                           std::vector<int>::const_iterator prefetch_start,
                                           std::vector<int>::const_iterator prefetch_end,
                                           unsigned long long int capacity,
                                           StorageBackend* backend, MetadataStore* metadata_store, int storage_level) :
                                           MemoryPrefetcher(backend_options, prefetch_start, prefetch_end, capacity, backend, metadata_store, storage_level, false) {
    path = backend_options["path"];

    fd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600); // NOLINT(hicpp-signed-bitwise)
    lseek(fd, capacity - 1, SEEK_SET);
    write(fd, "", 1);
    buffer = static_cast<char*>(mmap(nullptr, capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)); // NOLINT(hicpp-signed-bitwise)
}

FileSystemPrefetcher::~FileSystemPrefetcher() {
    munmap(buffer, capacity);
    close(fd);
    unlink(path.c_str());
}
