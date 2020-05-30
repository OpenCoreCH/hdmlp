cmake_minimum_required(VERSION 3.0)
project(hdmlp)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include/)

find_library(libconfig config++)
find_package(MPI REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Torch REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH} ${OpenCV_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_library(hdmlp SHARED src/utils/hdmlp_api.cpp src/storage/FileSystemBackend.cpp src/prefetcher/Prefetcher.cpp src/utils/Sampler.cpp src/utils/Configuration.cpp src/prefetcher/StagingBufferPrefetcher.cpp src/prefetcher/PrefetcherBackendFactory.cpp src/prefetcher/MemoryPrefetcher.cpp src/prefetcher/FileSystemPrefetcher.cpp src/utils/MetadataStore.cpp src/utils/DistributedManager.cpp src/transform/TransformPipeline.cpp src/transform/Transformation.cpp src/transform/ImgDecode.cpp src/transform/Resize.cpp src/transform/ToTensor.cpp)

target_link_libraries(hdmlp ${libconfig} ${MPI_C_LIBRARIES} ${OpenCV_LIBS} ${TORCH_LIBRARIES})
