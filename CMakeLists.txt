cmake_minimum_required(VERSION 3.15)
project(hdmlp)

set(CMAKE_CXX_STANDARD 14)

add_library(hdmlp SHARED src/hdmlp_api.cpp include/hdmlp_api.h include/StorageBackend.h src/FileSystemBackend.cpp include/FileSystemBackend.h src/Prefetcher.cpp include/Prefetcher.h src/Sampler.cpp include/Sampler.h)