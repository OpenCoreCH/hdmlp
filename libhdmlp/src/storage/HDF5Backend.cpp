#include <dirent.h>
#include <codecvt>
#include <locale>
#include <H5cpp.h>
#include <H5File.h>
#include <iostream>
#include <vector>
#include "../../include/storage/HDF5Backend.h"

HDF5Backend::HDF5Backend(const std::wstring& path, int node_id) {
    using type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<type, wchar_t> converter;
    std::string str_path = converter.to_bytes(path);
    if (str_path.back() != '/') {
        str_path += '/';
    }
    this->path = str_path;
    dataset_name_sample = "climate/data";
    dataset_name_label = "climate/labels_0";
}

int HDF5Backend::get_length() {
    return file_names.size();
}

unsigned long HDF5Backend::get_file_size(int file_id) {
    return sample_num_elems[file_id] * sample_elem_size;
}

void HDF5Backend::fetch(int file_id, char* dst) {
    H5::H5File file(path + file_names[file_id], H5F_ACC_RDONLY);
    H5::DataSet sample = file.openDataSet(dataset_name_sample);
    H5::DataSpace sample_dataspace = sample.getSpace();
    int sample_rank = sample_dataspace.getSimpleExtentNdims();
    hsize_t sample_dims[sample_rank];
    sample_dataspace.getSimpleExtentDims(sample_dims, NULL);
    H5::DataSpace memspace(sample_rank, sample_dims);
    sample.read(dst, H5::PredType::NATIVE_FLOAT, memspace, sample_dataspace);

}

void HDF5Backend::init_mappings() {
    std::vector<FileInformation> file_metadata;
    struct dirent* entry = nullptr;
    DIR* dp = nullptr;

    dp = opendir(this->path.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Invalid path specified");
    }
    while ((entry = readdir(dp))) {
        char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".h5") == 0) {
            FileInformation fi;
            fi.file_name = entry->d_name;
            file_metadata.emplace_back(fi);
            H5::H5File file(this->path + entry->d_name, H5F_ACC_RDONLY);
            H5::DataSet sample = file.openDataSet(dataset_name_sample);
            H5::DataSet label = file.openDataSet(dataset_name_label);
            H5T_class_t sample_type_class = sample.getTypeClass();
            H5T_class_t label_type_class = label.getTypeClass();
            if (sample_type_class == H5T_INTEGER) {
                sample_elem_size = sizeof(int);
            } else if (sample_type_class == H5T_FLOAT) {
                sample_elem_size = sizeof(float);
            } else {
                throw std::runtime_error("Unsupported HDF5 datatype");
            }
            if (label_type_class == H5T_INTEGER) {
                label_elem_size = sizeof(int);
            } else if (label_type_class == H5T_FLOAT) {
                label_elem_size = sizeof(float);
            } else {
                throw std::runtime_error("Unsupported HDF5 datatype");
            }
            H5::DataSpace sample_dataspace = sample.getSpace();
            H5::DataSpace label_dataspace = label.getSpace();
            int sample_rank = sample_dataspace.getSimpleExtentNdims();
            int label_rank = label_dataspace.getSimpleExtentNdims();
            hsize_t sample_dims[sample_rank];
            hsize_t label_dims[label_rank];
            sample_dataspace.getSimpleExtentDims(sample_dims, NULL);
            label_dataspace.getSimpleExtentDims(label_dims, NULL);
            int sample_size = 1;
            for (int i = 0; i < sample_rank; i++) {
                sample_size *= sample_dims[i];
            }
            int label_size = 1;
            for (int i = 0; i < label_rank; i++) {
                label_size *= label_dims[i];
            }
            fi.num_elems_sample = sample_size;
            fi.num_elems_label = label_size;
        }
    }
    std::sort(file_metadata.begin(), file_metadata.end(), [](FileInformation& a, FileInformation& b) {
                  return a.file_name > b.file_name;
              }
    );
    file_names.resize(file_metadata.size());
    sample_num_elems.resize(file_metadata.size());
    label_num_elems.resize(file_metadata.size());
    for (auto & i : file_metadata) {
        file_names.emplace_back(i.file_name);
        sample_num_elems.emplace_back(i.num_elems_sample);
        label_num_elems.emplace_back(i.num_elems_label);
    }
}

int HDF5Backend::get_label_size(int file_id) {
    return label_num_elems[file_id] * label_elem_size;
}

void HDF5Backend::fetch_label(int file_id, char* dst) {

}
