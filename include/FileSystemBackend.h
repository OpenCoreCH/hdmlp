#ifndef HDMLP_FILESYSTEMBACKEND_H
#define HDMLP_FILESYSTEMBACKEND_H


#include "StorageBackend.h"

class FileSystemBackend : public StorageBackend {
public:
    explicit FileSystemBackend(const std::wstring& path);
    std::map<int, std::string> get_id_mapping() override;
    int get_length() override;
    unsigned long get_entry_size(int file_id) override;
    void fetch(int file_id, char* dst, unsigned long entry_size_hint) override;
private:
    std::string path;
    std::map<int, std::string> label_mappings;
    std::map<int, int> size_mappings;
    std::map<int, std::string> id_mappings;

    void init_mappings();

    std::string abs_path(const std::string *rel_path);
};


#endif //HDMLP_FILESYSTEMBACKEND_H
