cmake_minimum_required(VERSION 3.0)
project(hdmlp)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include/)

find_library(libconfig config++)
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_library(hdmlp SHARED src/utils/hdmlp_api.cpp src/storage/FileSystemBackend.cpp src/prefetcher/Prefetcher.cpp src/utils/Sampler.cpp src/utils/Configuration.cpp src/prefetcher/StagingBufferPrefetcher.cpp src/prefetcher/PrefetcherBackendFactory.cpp src/prefetcher/MemoryPrefetcher.cpp src/prefetcher/FileSystemPrefetcher.cpp src/utils/MetadataStore.cpp src/utils/DistributedManager.cpp)

target_link_libraries(hdmlp ${libconfig} ${MPI_C_LIBRARIES})
