#ifndef HDMLP_SAMPLER_H
#define HDMLP_SAMPLER_H


#include <vector>
#include <random>
#include <map>

class Sampler {
public:
    Sampler(int count,
            int n,
            int batch_size,
            int distr_scheme,
            bool drop_last_batch,
            int seed);
private:
    std::vector<int>* access_sequence;
    std::default_random_engine* random_engine;
    int n;
    int count;
    int batch_size;
    int distr_scheme;
    int node_local_batch_size;
    int batch_no;


    void shuffle_sequence(std::vector<int>* vec, bool restore_random_state);
    void get_access_frequency(std::map<int, int>* access_freq, int node_id, int lookahead);
    void get_access_frequency_for_seq(std::vector<int>* seq, std::map<int, int>* access_freq, int node_id);

    void fisher_yates(std::vector<int> *vec);
};


#endif //HDMLP_SAMPLER_H
