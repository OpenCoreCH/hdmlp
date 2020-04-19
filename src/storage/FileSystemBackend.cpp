#include "../../include/storage/FileSystemBackend.h"
#include <dirent.h>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>


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

std::unordered_map<int, std::string> FileSystemBackend::get_id_mapping() {
    return id_mappings;
}

std::string FileSystemBackend::get_label(int file_id) {
    return label_mappings[file_id];
}

int FileSystemBackend::get_length() {
    return id_mappings.size();
}

void FileSystemBackend::init_mappings() {
    std::vector<FileInformation> file_information;
    struct dirent* entry = nullptr;
    DIR* dp = nullptr;

    dp = opendir(path.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Invalid path specified");
    }
    while ((entry = readdir(dp))) {
        if (entry->d_name[0] != '.') {
            std::string dir_name = entry->d_name;
            struct dirent* subentry = nullptr;
            DIR* subp = nullptr;

            subp = opendir((path + dir_name).c_str());
            if (subp == nullptr) {
                // Not a directory
                continue;
            }

            while ((subentry = readdir(subp))) {
                std::string rel_path = entry->d_name;
                rel_path += '/';
                rel_path += subentry->d_name;
                std::string file_name = abs_path(&rel_path);
                int fd = open(file_name.c_str(), O_RDONLY);
                struct stat stbuf; // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
                fstat(fd, &stbuf);
                close(fd);
                if (!S_ISREG(stbuf.st_mode)) { // NOLINT(hicpp-signed-bitwise)
                    // Not a regular file
                    continue;
                }
                struct FileInformation fi{};
                fi.label = entry->d_name;
                fi.file_name = subentry->d_name;
                fi.file_size = stbuf.st_size;
                file_information.push_back(fi);
            }

            closedir(subp);


        }
    }
    closedir(dp);
    // Ensure that all nodes have same file ids by sorting them
    std::sort(file_information.begin(), file_information.end(), [](FileInformation& a, FileInformation& b) {
                  return a.label + a.file_name > b.label + b.file_name;
              }
    );
    for (int i = 0; i < file_information.size(); i++) {
        FileInformation fi = file_information[i];
        id_mappings[i] = fi.file_name;
        label_mappings[i] = fi.label;
        size_mappings[i] = fi.file_size;
    }
}

std::string FileSystemBackend::abs_path(const std::string* rel_path) {
    return path + *rel_path;
}

unsigned long FileSystemBackend::get_file_size(int file_id) {
    return size_mappings[file_id];
}

/**
 * Fetch the file from the backend
 *
 * @param file_id
 * @param dst
 */
void FileSystemBackend::fetch(int file_id, char* dst) {
    std::string label = label_mappings[file_id];
    std::string rel_path = label + '/' + id_mappings[file_id];
    std::string file_name = abs_path(&rel_path);
    unsigned long entry_size = get_file_size(file_id);
    FILE* f = fopen(file_name.c_str(), "rb");
    fread(dst, 1, entry_size, f);
    fclose(f);
}
