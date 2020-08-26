#ifndef HDMLP_HDF5BACKEND_H
#define HDMLP_HDF5BACKEND_H


#include "StorageBackend.h"

class HDF5Backend : public StorageBackend {
public:
    HDF5Backend(const std::wstring& path, int node_id);

    std::string get_label(int file_id) override;

    int get_length() override;

    unsigned long get_file_size(int file_id) override;

    void fetch(int file_id, char* dst) override;

private:
    std::string path;

};


#endif //HDMLP_HDF5BACKEND_H
