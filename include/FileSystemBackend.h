#ifndef HDMLP_FILESYSTEMBACKEND_H
#define HDMLP_FILESYSTEMBACKEND_H


#include "StorageBackend.h"

class FileSystemBackend : public StorageBackend {
public:
    explicit FileSystemBackend(const std::wstring& path);
    std::map<int, std::string> get_id_mapping() override;
    int get_length() override;
    int get_file_size(int file_id) override;
    void fetch(int file_id, char* dst, int file_size_hint) override;
private:
    std::string path;
    std::map<int, std::string> id_mappings;

    void init_id_mapping();

    std::string abs_path(const std::string *rel_path);
};


#endif //HDMLP_FILESYSTEMBACKEND_H
