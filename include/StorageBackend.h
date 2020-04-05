#ifndef HDMLP_STORAGEBACKEND_H
#define HDMLP_STORAGEBACKEND_H


#include <string>
#include <map>

class StorageBackend {
public:
    virtual ~StorageBackend() = default;
    virtual int get_length() = 0;
    virtual std::map<int, std::string> get_id_mapping() = 0;
    virtual unsigned long get_entry_size(int file_id) = 0;
    virtual void fetch(int file_id, char* dst, unsigned long file_size_hint) = 0;
};


#endif //HDMLP_STORAGEBACKEND_H
