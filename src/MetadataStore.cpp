#include "../include/MetadataStore.h"

void MetadataStore::insert_cached_file(int storage_level, int file_id) {
    std::lock_guard<std::shared_timed_mutex> writer_lock(file_locations_mutex);
    file_locations[file_id] = storage_level;
}

int MetadataStore::get_storage_level(int file_id) {
    std::shared_lock<std::shared_timed_mutex> reader_lock(file_locations_mutex);
    if (file_locations.count(file_id) != 0) {
        return file_locations[file_id];
    } else {
        return 0;
    }
}
