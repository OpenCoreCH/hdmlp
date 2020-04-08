#ifndef HDMLP_PREFETCHERBACKEND_H
#define HDMLP_PREFETCHERBACKEND_H

class PrefetcherBackend {
public:
    virtual ~PrefetcherBackend() = default;
    virtual void prefetch() = 0;
};

#endif //HDMLP_PREFETCHERBACKEND_H
