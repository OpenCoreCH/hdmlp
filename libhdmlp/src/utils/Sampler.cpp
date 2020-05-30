#include <iostream>
#include <algorithm>
#include "../../include/utils/Sampler.h"

Sampler::Sampler(StorageBackend* backend, // NOLINT(cert-msc32-c,cert-msc51-cpp)
                 int n,
                 int batch_size,
                 int epochs,
                 int distr_scheme,
                 bool drop_last_batch,
                 int seed) {
    count = backend->get_length();
    access_sequence.resize(count);
    for (int i = 0; i < count; i++) {
        access_sequence[i] = i;
    }
    this->backend = backend;
    this->n = n;
    this->batch_size = batch_size;
    this->distr_scheme = distr_scheme;
    this->epochs = epochs;
    if (drop_last_batch) {
        batch_no = count / batch_size;
    } else {
        batch_no = count / batch_size + (count % batch_size != 0);
    }
    node_local_batch_size = batch_size / n + (batch_size % n != 0);

    random_engine.seed(seed);
    shuffle_sequence(&access_sequence);
}

/**
 * Shuffle the provided sequence (vector).
 *
 * @param vec Pointer to vector that is shuffled
 */
void Sampler::shuffle_sequence(std::vector<int>* vec) {
    std::shuffle(vec->begin(), vec->end(), random_engine);
}

/**
 * Gets the access string for a given node in the current epoch
 * @param node_id
 * @param access_string
 */
void Sampler::get_node_access_string(int node_id, std::vector<int>* access_string) {
    get_node_access_string_for_seq(&access_sequence, node_id, access_string);
}

void Sampler::get_node_access_string_for_seq(std::vector<int>* seq, int node_id, std::vector<int>* access_string) {
    switch (distr_scheme) {
        case 1: {
            int offset = node_local_batch_size * node_id;
            for (int j = 0; j < batch_no; j++) {
                for (int k = j * batch_size + offset;
                     k < std::min(j * batch_size + std::min(offset + node_local_batch_size, batch_size), count);
                     k++) {
                    int file_id = (*seq)[k];
                    access_string->push_back(file_id);
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Unsupported distr_scheme");
    }
}

void Sampler::get_access_frequency(std::map<int, int>* access_freq, int node_id, int lookahead) {
    std::default_random_engine engine_copy = random_engine;
    std::vector<int> curr_access_seq = access_sequence;
    get_access_frequency_for_seq(&curr_access_seq, access_freq, node_id);
    for (int i = 1; i < lookahead; i++) {
        shuffle_sequence(&curr_access_seq);
        get_access_frequency_for_seq(&curr_access_seq, access_freq, node_id);
    }
    random_engine = engine_copy;
}


void Sampler::get_prefetch_string(int node_id, const std::vector<unsigned long long int>* capacities,
                                  std::vector<int>* prefetch_string,
                                  std::vector<std::vector<int>::iterator>* storage_class_ends, bool in_order) {
    std::map<int, int> access_freq;
    get_access_frequency(&access_freq, node_id, epochs);
    std::vector<std::pair<int, int>> access_freq_vec;
    access_freq_vec.reserve(access_freq.size());
    for (const auto& pair : access_freq) {
        access_freq_vec.emplace_back(pair);
    }
    std::sort(access_freq_vec.begin(), access_freq_vec.end(), [](std::pair<int, int>& a, std::pair<int, int>& b) {
                  return a.second > b.second;
              }
    );
    prefetch_string->reserve(access_freq_vec.size());
    unsigned long long curr_size = 0;
    int num_storage_classes = capacities->size();
    int curr_storage_class = 1;
    for (const auto& pair : access_freq_vec) {
        unsigned long size = backend->get_file_size(pair.first);
        if (curr_size + size > (*capacities)[curr_storage_class]) {
            storage_class_ends->push_back(prefetch_string->end());
            if (curr_storage_class < num_storage_classes - 1) {
                curr_storage_class++;
                curr_size = 0;
            } else {
                break;
            }
        }
        prefetch_string->emplace_back(pair.first);
        curr_size += size;
    }
    if ((int) storage_class_ends->size() < num_storage_classes - 1) {
        storage_class_ends->push_back(prefetch_string->end());
    }
    if (in_order) {
        std::map<int, int> first_accesses;
        get_first_accesses(&first_accesses, node_id, epochs);
        auto storage_class_begin = prefetch_string->begin();
        for (auto& storage_class_end : *storage_class_ends) {
            std::sort(storage_class_begin, storage_class_end, [&first_accesses](int& a, int& b) {
                          if (first_accesses.count(a) == 0) {
                              return false;
                          }
                          if (first_accesses.count(b) == 0) {
                              return true;
                          }
                          return first_accesses[a] < first_accesses[b];
                      }
            );
            storage_class_begin = storage_class_end;
        }
    }
}

void Sampler::get_access_frequency_for_seq(std::vector<int>* seq, std::map<int, int>* access_freq, int node_id) {
    std::vector<int> access_string;
    get_node_access_string_for_seq(seq, node_id, &access_string);
    for (const auto& file_id : access_string) {
        if (access_freq->count(file_id)) {
            (*access_freq)[file_id]++;
        } else {
            (*access_freq)[file_id] = 1;
        }
    }
}

void Sampler::advance_batch() {
    shuffle_sequence(&access_sequence);
}

void Sampler::get_first_accesses(std::map<int, int>* first_accesses, int node_id, int lookahead) {
    std::default_random_engine engine_copy = random_engine;
    std::vector<int> curr_access_seq = access_sequence;
    int offset = 0;
    for (int i = 0; i < lookahead; i++) {
        std::vector<int> access_string;
        get_node_access_string_for_seq(&curr_access_seq, node_id, &access_string);
        for (int file_id : access_string) {
            if (first_accesses->count(file_id) == 0) {
                (*first_accesses)[file_id] = offset;
            }
            offset++;
        }
        if (i != lookahead - 1) {
            shuffle_sequence(&curr_access_seq);
        }
    }
    random_engine = engine_copy;
}
