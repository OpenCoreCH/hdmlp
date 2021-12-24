# HDMLP: Hierarchical Distributed Machine Learning Prefetcher
![HDMLP](hdmlp.png?raw=true "Title")

## Installation
Using setuptools:
- Clone this repo using ```git clone```
- Run ```python setup.py install```

### Non-default libconfig location
If libconfig is installed in a non-default location (`$libconfig_install_location`), you need to export the following environment variables before running the install script:
```
export CMAKE_PREFIX_PATH=$libconfig_install_location:$CMAKE_PREFIX_PATH
export CXXFLAGS=-isystem\ $libconfig_install_location/include/
```

### Non-default OpenCV location
If OpenCV is installed in a non-default location (`$opencv_install_location`), the environment variable `OpenCV_DIR` needs to be set to:
```
export OpenCV_DIR=$opencv_install_location/lib64/cmake/opencv4/
``` 

## Requirements

- Python 3
- Any MPI implementation (e.g. OpenMPI)
- cmake
- libconfig
- OpenCV

## Documentation
The system is documented in [OpenCoreCH/Clairvoyant-Prefetching-for-Machine-Learning-I-O](https://github.com/OpenCoreCH/Clairvoyant-Prefetching-for-Machine-Learning-I-O)

## License
HDMLP is published under the new BSD license, see [LICENSE](LICENSE)
