#ifndef HDMLP_STORAGEBACKEND_H
#define HDMLP_STORAGEBACKEND_H


#include <string>
#include <map>

class StorageBackend {
public:
    virtual int get_length() = 0;
    virtual std::map<int, std::string> get_id_mapping() = 0;
};


#endif //HDMLP_STORAGEBACKEND_H
