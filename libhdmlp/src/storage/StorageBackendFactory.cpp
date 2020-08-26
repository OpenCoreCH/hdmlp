#include "../../include/storage/StorageBackendFactory.h"
#include "../../include/storage/FileSystemBackend.h"
#include "../../include/storage/HDF5Backend.h"

StorageBackend*
StorageBackendFactory::create(const std::string& storage_backend, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>& dataset_path, bool checkpoint, const std::string& checkpoint_path,
                              int node_id) {
    if (storage_backend == "filesystem") {
        return new FileSystemBackend(dataset_path, checkpoint, checkpoint_path, node_id);
    } else if (storage_backend == "hdf5") {
        return new HDF5Backend(dataset_path, node_id);
    } else {
        throw std::runtime_error("Unsupported storage backend");
    }
}
