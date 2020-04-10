#include "../include/MetadataStore.h"

void MetadataStore::insert_cached_file(int storage_level, int file_id) {
    file_locations[file_id] = storage_level;
}

int MetadataStore::get_storage_level(int file_id) {
    if (file_locations.count(file_id) != 0) {
        return file_locations[file_id];
    } else {
        return 0;
    }
}
