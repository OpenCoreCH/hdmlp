#ifndef HDMLP_FILESYSTEMBACKEND_H
#define HDMLP_FILESYSTEMBACKEND_H


#include "StorageBackend.h"

class FileSystemBackend : public StorageBackend {
public:
    explicit FileSystemBackend(const std::wstring& path);

    std::string get_label(int file_id) override;

    int get_length() override;

    unsigned long get_file_size(int file_id) override;

    void fetch(int file_id, char* dst) override;

private:
    struct FileInformation {
        std::string label;
        std::string file_name;
        int file_size;
    };
    std::string path;
    std::unordered_map<int, std::string> label_mappings;
    std::unordered_map<int, int> size_mappings;
    std::unordered_map<int, std::string> id_mappings;

    void init_mappings();

    std::string abs_path(const std::string* rel_path);
};


#endif //HDMLP_FILESYSTEMBACKEND_H
