import ctypes
import pathlib
from typing import Optional


class Job:

    DISTR_SCHEMES = {'uniform': 1}

    def __init__(self,
                 dataset_path: str,
                 batch_size: int,
                 distr_scheme: str,
                 drop_last_batch: bool,
                 seed: Optional[int] = None):
        libname = pathlib.Path().absolute() / "../../cpp/hdmlp/cmake-build-debug/libhdmlp.dylib"
        self.hdmlp_lib = ctypes.CDLL(libname)
        self.dataset_path = dataset_path
        self.batch_size = batch_size
        if distr_scheme not in self.DISTR_SCHEMES:
            raise ValueError("Distribution scheme {} not supported".format(distr_scheme))
        self.distr_scheme = self.DISTR_SCHEMES[distr_scheme]
        self.drop_last_batch = drop_last_batch
        self.seed = seed

    def setup(self):
        self.hdmlp_lib.setup(ctypes.c_wchar_p(self.dataset_path),
                             self.batch_size,
                             self.distr_scheme,
                             ctypes.c_bool(self.drop_last_batch),
                             self.seed)

    def get(self):
        pass
