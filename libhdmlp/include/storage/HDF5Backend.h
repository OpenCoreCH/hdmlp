#ifndef HDMLP_HDF5BACKEND_H
#define HDMLP_HDF5BACKEND_H


#include "StorageBackend.h"

class HDF5Backend : public StorageBackend {
public:
    HDF5Backend(const std::wstring& path, int node_id);

    void fetch_label(int file_id, char* dst) override;

    virtual int get_label_size(int file_id) override;

    int get_length() override;

    unsigned long get_file_size(int file_id) override;

    void fetch(int file_id, char* dst) override;

private:
    std::string path;
    std::string dataset_name_sample;
    std::string dataset_name_label;
    struct FileInformation {
        std::string file_name;
        int num_elems_sample;
        int num_elems_label;
    };
    int sample_elem_size = 0;
    int label_elem_size = 0;
    std::vector<std::string> file_names;
    std::vector<int> sample_num_elems;
    std::vector<int> label_num_elems;

    void init_mappings();
};


#endif //HDMLP_HDF5BACKEND_H
