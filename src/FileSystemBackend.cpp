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
    this->init_mappings();
}

std::map<int, std::string> FileSystemBackend::get_id_mapping() {
    return id_mappings;
}

int FileSystemBackend::get_length() {
    return id_mappings.size();
}

void FileSystemBackend::init_mappings() {
    int id = 0;
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    dp = opendir(path.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Invalid path specified");
    }
    while ((entry = readdir(dp))) {
        if (entry->d_name[0] != '.' && entry->d_type == DT_DIR) {
            std::string dir_name = entry->d_name;
            struct dirent *subentry = nullptr;
            DIR *subp = nullptr;

            subp = opendir((path + dir_name).c_str());

            while ((subentry = readdir(subp))) {
                if (subentry->d_type == DT_REG) {
                    label_mappings[id] = entry->d_name;
                    id_mappings[id] = subentry->d_name;
                    std::string rel_path = label_mappings[id] + '/' + id_mappings[id];
                    std::string file_name = abs_path(&rel_path);
                    int fd = open(file_name.c_str(), O_RDONLY);
                    struct stat stbuf; // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
                    fstat(fd, &stbuf);
                    close(fd);
                    size_mappings[id] = stbuf.st_size + label_mappings[id].size() + 1;
                    id += 1;
                }
            }

            closedir(subp);


        }
    }
    closedir(dp);
}

std::string FileSystemBackend::abs_path(const std::string* rel_path) {
    return path + *rel_path;
}

unsigned long FileSystemBackend::get_entry_size(int file_id) {
    return size_mappings[file_id];
}

/**
 * Fetch the file from the backend
 *
 * @param file_id
 * @param dst
 * @param file_size_hint Optional hint indicating the file size (if known by the producer). '-1' if unknown
 */
void FileSystemBackend::fetch(int file_id, char *dst, unsigned long file_size_hint) {
    std::string label = label_mappings[file_id];
    std::string rel_path = label + '/' + id_mappings[file_id];
    std::string file_name = abs_path(&rel_path);
    strcpy(dst, label.c_str());
    unsigned long file_size = file_size_hint;
    if (file_size_hint == -1) {
        file_size = get_entry_size(file_id);
    }
    FILE* f = fopen(file_name.c_str(), "rb");
    fread(dst + label.length() + 1, 1, file_size, f);
    fclose(f);
}
