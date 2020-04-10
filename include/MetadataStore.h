#ifndef HDMLP_METADATASTORE_H
#define HDMLP_METADATASTORE_H


#include <unordered_map>

class MetadataStore {
public:

    void insert_cached_file(int storage_level, int file_id);
    int get_storage_level(int file_id);

private:
    std::unordered_map<int, int> file_locations;
};


#endif //HDMLP_METADATASTORE_H
