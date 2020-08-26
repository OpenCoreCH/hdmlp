cmake_minimum_required(VERSION 3.0)
project(hdmlp)

set(CMAKE_CXX_STANDARD 14)

find_library(libconfig config++)
find_package(MPI REQUIRED)
find_package(OpenCV REQUIRED)
option(ENABLE_HDF5 "Enable HDF5 support" ON)
if(ENABLE_HDF5)
    find_package(HDF5 COMPONENTS CXX)
    include_directories(${HDF5_INCLUDE_DIR})
    set(_hdf5_libs hdf5 hdf5_cpp)
endif()
include_directories(SYSTEM ${MPI_INCLUDE_PATH} ${OpenCV_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_library(hdmlp SHARED src/utils/hdmlp_api.cpp src/storage/FileSystemBackend.cpp src/prefetcher/Prefetcher.cpp src/utils/Sampler.cpp src/utils/Configuration.cpp src/prefetcher/StagingBufferPrefetcher.cpp src/prefetcher/PrefetcherBackendFactory.cpp src/prefetcher/MemoryPrefetcher.cpp src/prefetcher/FileSystemPrefetcher.cpp src/utils/MetadataStore.cpp src/utils/DistributedManager.cpp src/transform/TransformPipeline.cpp src/transform/Transformation.cpp src/transform/ImgDecode.cpp src/transform/Resize.cpp src/transform/ToTensor.cpp src/transform/RandomHorizontalFlip.cpp src/transform/RandomVerticalFlip.cpp src/transform/RandomResizedCrop.cpp src/transform/Normalize.cpp src/transform/CenterCrop.cpp src/storage/HDF5Backend.cpp include/storage/HDF5Backend.h src/storage/StorageBackendFactory.cpp include/storage/StorageBackendFactory.h)

target_link_libraries(hdmlp ${libconfig} ${MPI_C_LIBRARIES} ${OpenCV_LIBS})
if(ENABLE_HDF5)
    target_link_libraries(hdmlp ${HDF5_LIBRARIES})
endif()