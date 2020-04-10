#include <iostream>
#include "../include/FileSystemPrefetcher.h"

FileSystemPrefetcher::FileSystemPrefetcher(const std::map<std::string, std::string> &backend_options,
                                           const std::vector<int>::const_iterator prefetch_start,
                                           const std::vector<int>::const_iterator prefetch_end) {

}

void FileSystemPrefetcher::prefetch() {
    std::cout << "FileSystemPrefetcher::prefetch called" << std::endl;
}
