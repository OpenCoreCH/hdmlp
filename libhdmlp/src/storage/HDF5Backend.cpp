#include <dirent.h>
#include <codecvt>
#include <locale>
#include <H5cpp.h>
#include <H5File.h>
#include <iostream>
#include "../../include/storage/HDF5Backend.h"

HDF5Backend::HDF5Backend(const std::wstring& path, int node_id) {
    using type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<type, wchar_t> converter;
    std::string str_path = converter.to_bytes(path);
    if (str_path.back() != '/') {
        str_path += '/';
    }
    this->path = str_path;
    struct dirent* entry = nullptr;
    DIR* dp = nullptr;

    dp = opendir(this->path.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Invalid path specified");
    }
    while ((entry = readdir(dp))) {
        char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".h5") == 0) {
            std::cout << this->path + entry->d_name << std::endl;
            H5::H5File file(this->path + entry->d_name, H5F_ACC_RDONLY);
        }
    }

}

std::string HDF5Backend::get_label(int file_id) {
    return std::string();
}

int HDF5Backend::get_length() {
    return 0;
}

unsigned long HDF5Backend::get_file_size(int file_id) {
    return 0;
}

void HDF5Backend::fetch(int file_id, char* dst) {

}
