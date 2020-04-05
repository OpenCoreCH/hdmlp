import ctypes
import pathlib
from typing import Optional


class Job:

    DISTR_SCHEMES = {'uniform': 1}

    def __init__(self,
                 dataset_path: str,
                 batch_size: int,
                 epochs: int,
                 distr_scheme: str,
                 drop_last_batch: bool,
                 seed: Optional[int] = None):
        libname = pathlib.Path().absolute() / "../../cpp/hdmlp/cmake-build-debug/libhdmlp.dylib"
        self.hdmlp_lib = ctypes.CDLL(libname)
        self.dataset_path = dataset_path
        self.batch_size = batch_size
        self.epochs = epochs
        if distr_scheme not in self.DISTR_SCHEMES:
            raise ValueError("Distribution scheme {} not supported".format(distr_scheme))
        self.distr_scheme = self.DISTR_SCHEMES[distr_scheme]
        self.drop_last_batch = drop_last_batch
        self.seed = seed
        self.buffer_p = None
        self.buffer_offset = 0

    def setup(self):
        self.hdmlp_lib.setup.restype = ctypes.c_char_p
        buffer = self.hdmlp_lib.setup(ctypes.c_wchar_p(self.dataset_path),
                             self.batch_size,
                             self.epochs,
                             self.distr_scheme,
                             ctypes.c_bool(self.drop_last_batch),
                             self.seed)
        self.buffer_p = ctypes.cast(buffer, ctypes.POINTER(ctypes.c_char))

    def destroy(self):
        self.hdmlp_lib.destroy()

    def get(self):
        file_end = self.hdmlp_lib.get_next_file_end()
        #print(file_end)
        if file_end < self.buffer_offset:
            self.buffer_offset = 0
        file = self.buffer_p[self.buffer_offset:file_end]
        self.buffer_offset = file_end
        return file
