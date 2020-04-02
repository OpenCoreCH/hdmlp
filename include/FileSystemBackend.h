#ifndef HDMLP_FILESYSTEMBACKEND_H
#define HDMLP_FILESYSTEMBACKEND_H


#include "StorageBackend.h"

class FileSystemBackend : public StorageBackend {
public:
    explicit FileSystemBackend(const std::wstring& path);
    std::map<int, std::string> get_id_mapping() override;
    int get_length() override;
private:
    std::wstring path;
    std::map<int, std::string> id_mappings;

    void init_id_mapping();
};


#endif //HDMLP_FILESYSTEMBACKEND_H
