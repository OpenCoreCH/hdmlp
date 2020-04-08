#include <iostream>
#include "../include/FileSystemPrefetcher.h"

FileSystemPrefetcher::FileSystemPrefetcher(const std::map<std::string, std::string>& backend_options) {

}

void FileSystemPrefetcher::prefetch() {
    std::cout << "FileSystemPrefetcher::prefetch called" << std::endl;
}
