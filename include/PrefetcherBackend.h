#ifndef HDMLP_PREFETCHERBACKEND_H
#define HDMLP_PREFETCHERBACKEND_H

class PrefetcherBackend {
public:
    virtual ~PrefetcherBackend() = default;
    virtual void prefetch() = 0;
    virtual void fetch(int file_id, char *dst) = 0;
};

#endif //HDMLP_PREFETCHERBACKEND_H
