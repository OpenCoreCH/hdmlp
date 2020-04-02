#include "../include/FileSystemBackend.h"
#include <dirent.h>
#include <codecvt>
#include <locale>
#include <iostream>

FileSystemBackend::FileSystemBackend(const std::wstring& path) {
    this->path = path;
    this->init_id_mapping();
}

std::map<int, std::string> FileSystemBackend::get_id_mapping() {
    return id_mappings;
}

int FileSystemBackend::get_length() {
    return id_mappings.size();
}

void FileSystemBackend::init_id_mapping() {
    int id = 0;
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    using type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<type, wchar_t> converter;
    std::string str_path = converter.to_bytes(path);

    dp = opendir(str_path.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Invalid path specified");
    }
    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) {
            id_mappings.insert(std::pair<int, std::string>(id, entry->d_name));
            std::cout << entry->d_name << std::endl;
        }
    }
    closedir(dp);
}