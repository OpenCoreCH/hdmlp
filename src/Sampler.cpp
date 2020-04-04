#include <iostream>
#include "../include/Sampler.h"

Sampler::Sampler(int count,
                 int n,
                 int batch_size,
                 int distr_scheme,
                 bool drop_last_batch,
                 int seed) {
    access_sequence = new std::vector<int>(count);
    for (int i = 0; i < count; i++) {
        (*access_sequence)[i] = i;
    }
    if (seed == 0) {
        seed = std::chrono::system_clock::now().time_since_epoch().count();
    }
    this->n = n;
    this->count = count;
    this->batch_size = batch_size;
    this->distr_scheme = distr_scheme;
    if (drop_last_batch) {
        batch_no = count / batch_size;
    } else {
        batch_no = count / batch_size + (count % batch_size != 0);
    }
    node_local_batch_size = batch_size / n + (batch_size % n != 0);

    random_engine = new std::default_random_engine(seed);
    shuffle_sequence(access_sequence, false);
    std::map<int, int> access_freq;
    get_access_frequency(&access_freq, 0, 10);
    for (const auto &pair : access_freq) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }
}

/**
 * Shuffle the provided sequence (vector).
 *
 * @param vec Pointer to vector that is shuffled
 * @param restore_random_state If this param is set to true, random_engine will be restored after shuffling. This is used
 * for looking ahead without changing the random state (i.e. for getting the access frequency in the beginning)
 */
void Sampler::shuffle_sequence(std::vector<int>* vec, bool restore_random_state) {
    if (restore_random_state) {
        std::default_random_engine engine_copy = *random_engine;
        std::shuffle(vec->begin(), vec->end(), *random_engine);
        *random_engine = engine_copy;
    } else {
        std::shuffle(vec->begin(), vec->end(), *random_engine);
    }
}

void Sampler::get_access_frequency(std::map<int, int>* access_freq, int node_id, int lookahead) {
    std::vector<int> curr_access_seq = *access_sequence;
    get_access_frequency_for_seq(&curr_access_seq, access_freq, node_id);
    for (int i = 1; i < lookahead; i++) {
        shuffle_sequence(&curr_access_seq, true);
        get_access_frequency_for_seq(&curr_access_seq, access_freq, node_id);
    }
}

void Sampler::get_access_frequency_for_seq(std::vector<int>* seq, std::map<int, int>* access_freq, int node_id) {
    switch (distr_scheme) {
        case 1:
        {
            int offset = node_local_batch_size * node_id;
            for (int j = 0; j < batch_no; j++) {
                for (int k = j * batch_size + offset;
                     k < std::min(j * batch_size + std::min(offset + node_local_batch_size, batch_size), count);
                     k++) {
                    int file_id = (*seq)[k];
                    if (access_freq->count(file_id)) {
                        (*access_freq)[file_id] += 1;
                    } else {
                        (*access_freq)[file_id] = 1;
                    }
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Unsupported distr_scheme");
    }

}