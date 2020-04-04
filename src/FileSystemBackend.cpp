#include "../include/FileSystemBackend.h"
#include <dirent.h>
#include <codecvt>
#include <locale>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>


FileSystemBackend::FileSystemBackend(const std::wstring& path) {
    using type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<type, wchar_t> converter;
    std::string str_path = converter.to_bytes(path);
    if (str_path.back() != '/') {
        str_path += '/';
    }
    this->path = str_path;
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

    dp = opendir(path.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Invalid path specified");
    }
    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) {
            id_mappings[id] = entry->d_name;
            id += 1;
        }
    }
    closedir(dp);
}

std::string FileSystemBackend::abs_path(const std::string* rel_path) {
    return path + *rel_path;
}

int FileSystemBackend::get_file_size(int file_id) {
    std::string file_name = abs_path(&id_mappings[file_id]);
    int fd = open(file_name.c_str(), O_RDONLY);
    struct stat stbuf; // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    fstat(fd, &stbuf);
    close(fd);
    return stbuf.st_size;
}

/**
 * Fetch the file from the backend
 *
 * @param file_id
 * @param dst
 * @param file_size_hint Optional hint indicating the file size (if known by the producer). '-1' if unknown
 */
void FileSystemBackend::fetch(int file_id, char *dst, int file_size_hint) {
    std::string file_name = abs_path(&id_mappings[file_id]);
    int file_size = file_size_hint;
    if (file_size_hint == -1) {
        file_size = get_file_size(file_id);
    }
    FILE* f = fopen(file_name.c_str(), "rb");
    fread(dst, 1, file_size, f);
    fclose(f);
}
