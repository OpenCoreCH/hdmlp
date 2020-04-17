cmake_minimum_required(VERSION 3.0)
project(hdmlp)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include/)

find_library(libconfig config++)
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_library(hdmlp SHARED src/hdmlp_api.cpp include/hdmlp_api.h include/StorageBackend.h src/FileSystemBackend.cpp include/FileSystemBackend.h src/Prefetcher.cpp include/Prefetcher.h src/Sampler.cpp include/Sampler.h src/Configuration.cpp include/Configuration.h src/StagingBufferPrefetcher.cpp include/StagingBufferPrefetcher.h include/PrefetcherBackend.h src/PrefetcherBackendFactory.cpp include/PrefetcherBackendFactory.h src/MemoryPrefetcher.cpp include/MemoryPrefetcher.h src/FileSystemPrefetcher.cpp include/FileSystemPrefetcher.h src/MetadataStore.cpp include/MetadataStore.h src/DistributedManager.cpp include/DistributedManager.h)

target_link_libraries(hdmlp ${libconfig} ${MPI_C_LIBRARIES})
