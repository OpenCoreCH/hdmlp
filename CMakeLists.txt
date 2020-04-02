cmake_minimum_required(VERSION 3.15)
project(hdmlp)

set(CMAKE_CXX_STANDARD 14)

add_library(hdmlp SHARED src/hdmlp_api.cpp include/hdmlp_api.h src/StorageBackend.cpp include/StorageBackend.h src/FileSystemBackend.cpp include/FileSystemBackend.h)