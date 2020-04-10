#include <iostream>
#include "../include/FileSystemPrefetcher.h"
#include "../include/MetadataStore.h"

FileSystemPrefetcher::FileSystemPrefetcher(const std::map<std::string,
        std::string> &backend_options, std::vector<int>::const_iterator prefetch_start,
                                           std::vector<int>::const_iterator prefetch_end,
                                           unsigned long long int capacity,
                                           StorageBackend* backend, MetadataStore* metadata_store, int storage_level) {

}

void FileSystemPrefetcher::prefetch() {
    std::cout << "FileSystemPrefetcher::prefetch called" << std::endl;
}

void FileSystemPrefetcher::fetch(int file_id, char *dst) {

}
