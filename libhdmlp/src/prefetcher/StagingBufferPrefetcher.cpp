#include <iostream>
#include <thread>
#include <cstring>
#include "../../include/prefetcher/StagingBufferPrefetcher.h"

StagingBufferPrefetcher::StagingBufferPrefetcher(char* staging_buffer, unsigned long long int buffer_size, int node_id,
                                                 int no_threads,
                                                 Sampler* sampler, StorageBackend* backend,
                                                 PrefetcherBackend** pf_backends,
                                                 MetadataStore* metadata_store, DistributedManager* distr_manager, TransformPipeline* transform_pipeline,
                                                 int transform_output_size) {
    this->buffer_size = buffer_size;
    this->staging_buffer = staging_buffer;
    this->node_id = node_id;
    this->no_threads = no_threads;
    this->sampler = new Sampler(*sampler);
    this->backend = backend;
    this->pf_backends = pf_backends;
    this->metadata_store = metadata_store;
    this->distr_manager = distr_manager;
    this->transform_pipeline = transform_pipeline;
    if (transform_pipeline != nullptr) {
        unsigned long max_file_size = 0;
        for (int i = 0; i < backend->get_length(); i++) {
            unsigned long size = backend->get_file_size(i);
            if (size > max_file_size) {
                max_file_size = size;
            }
        }
        transform_buffers = new char*[no_threads];
        for (int i = 0; i < no_threads; i++) {
            transform_buffers[i] = new char[max_file_size];
        }
        this->transform_output_size = transform_output_size;
    }
    global_batch_done = new bool[no_threads]();
}

StagingBufferPrefetcher::~StagingBufferPrefetcher() {
    delete sampler;
    delete[] global_batch_done;
    if (transform_pipeline != nullptr) {
        for (int i = 0; i < no_threads; i++) {
            delete[] transform_buffers[i];
        }
        delete[] transform_buffers;
    }
}

void StagingBufferPrefetcher::prefetch(int thread_id) {
    //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    while (true) {
        std::vector<int> curr_access_string;
        sampler->get_node_access_string(node_id, &curr_access_string);
        int batch_size = curr_access_string.size();
        while (true) {
            std::unique_lock<std::mutex> crit_section_lock(prefetcher_mutex);
            while (waiting_for_consumption) {
                consumption_waiting_cond_var.wait(crit_section_lock);
            }
            int j = prefetch_offset;
            if (j == 0) {
                curr_iter_file_ends.resize(batch_size);
                curr_iter_file_ends_ready.resize(batch_size);
                for (int i = 0; i < batch_size; i++) {
                    curr_iter_file_ends_ready[i] = false;
                }
            }
            prefetch_offset += 1;

            if (j >= batch_size) {
                break;
            }
            int file_id = curr_access_string[j];
            unsigned long file_size = backend->get_file_size(file_id);
            std::string label = backend->get_label(file_id);
            unsigned long entry_size = file_size + label.size() + 1;
            if (transform_pipeline != nullptr) {
                entry_size = transform_output_size + label.size() + 1;
            }
            while (staging_buffer_pointer < read_offset && staging_buffer_pointer + entry_size >= read_offset) {
                // Prevent overwriting of non-read data
                waiting_for_consumption = true;
                read_offset_cond_var.wait(crit_section_lock);
            }

            if (staging_buffer_pointer + entry_size > buffer_size) {
                // Start again at beginning of array
                staging_buffer_pointer = 0;
                waiting_for_consumption = true;
                // Ensure that overwriting is not possible after reset of pointer
                while (entry_size >= read_offset) {
                    read_offset_cond_var.wait(crit_section_lock);
                }
            }
            unsigned long long int local_staging_buffer_pointer = staging_buffer_pointer;
            staging_buffer_pointer += entry_size;
            if (waiting_for_consumption) {
                waiting_for_consumption = false;
                consumption_waiting_cond_var.notify_all();
            }
            crit_section_lock.unlock();


            strcpy(staging_buffer + local_staging_buffer_pointer, label.c_str());
            if (transform_pipeline == nullptr) {
                fetch(file_id, staging_buffer + local_staging_buffer_pointer + label.size() + 1, thread_id);
            } else {

                fetch(file_id, transform_buffers[thread_id], thread_id);
                transform_pipeline->transform(transform_buffers[thread_id], file_size, staging_buffer + local_staging_buffer_pointer + label.size() + 1);
            }
            std::unique_lock<std::mutex> staging_buffer_lock(staging_buffer_mutex);
            // Check if all the previous file ends were inserted to the queue. If not, don't insert, but only set
            // curr_iter_file_ends / curr_iter_file_ends_ready s.t. another thread will insert it
            curr_iter_file_ends[j] = local_staging_buffer_pointer + entry_size;
            curr_iter_file_ends_ready[j] = true;
            bool all_prev_inserted = true;
            for (int k = 0; k < j; k++) {
                if (!curr_iter_file_ends_ready[k]) {
                    all_prev_inserted = false;
                    break;
                }
            }
            if (all_prev_inserted) {
                // Also insert file_ends from faster threads
                int k = j;
                while (k < batch_size && curr_iter_file_ends_ready[k]) {
                    file_ends.push_back(curr_iter_file_ends[k]);
                    k++;
                }
                staging_buffer_cond_var.notify_one();
            }
            staging_buffer_lock.unlock();
        }
        bool all_threads_done = true;

        // Advance batch when all threads are done with the current one
        std::unique_lock<std::mutex> crit_section_lock(prefetcher_mutex);
        global_batch_done[thread_id] = true;
        for (int i = 0; i < no_threads; i++) {
            if (!global_batch_done[i]) {
                all_threads_done = false;
            }
        }
        if (all_threads_done) {
            sampler->advance_batch();
            prefetch_batch += 1;
            prefetch_offset = 0;
            for (int i = 0; i < no_threads; i++) {
                global_batch_done[i] = false;
            }
            batch_advancement_cond_var.notify_all();
        } else {
            batch_advancement_cond_var.wait(crit_section_lock);
        }

        if (prefetch_batch >= sampler->epochs) {
            break;
        }

        crit_section_lock.unlock();
    }
}

void StagingBufferPrefetcher::fetch(int file_id, char* dst, int thread_id) {
    int remote_storage_level = distr_manager->get_remote_storage_class(file_id);
    int local_storage_level = metadata_store->get_storage_level(file_id);
    int option_order[3];
    metadata_store->get_option_order(local_storage_level, remote_storage_level, option_order);
    if (option_order[0] == OPTION_REMOTE) {
        if (distr_manager->fetch(file_id, dst, thread_id)) {
            return;
        }
    }
    if (option_order[0] == OPTION_LOCAL || (option_order[0] == OPTION_REMOTE && option_order[1] == OPTION_LOCAL)) {
        pf_backends[local_storage_level - 1]->fetch(file_id, dst);
    } else {
        backend->fetch(file_id, dst);
    }
}

void StagingBufferPrefetcher::advance_read_offset(unsigned long long int new_offset) {
    std::unique_lock<std::mutex> lock(prefetcher_mutex);
    read_offset = new_offset;
    read_offset_cond_var.notify_one();
}

unsigned long long int StagingBufferPrefetcher::get_next_file_end() {
    std::unique_lock<std::mutex> staging_buffer_lock(staging_buffer_mutex);
    while (file_ends.empty()) {
        staging_buffer_cond_var.wait(staging_buffer_lock);
    }
    unsigned long long int file_end = file_ends.front();
    file_ends.pop_front();
    return file_end;
}
