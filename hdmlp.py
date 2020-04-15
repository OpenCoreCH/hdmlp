import ctypes
import pathlib
from sys import platform
from typing import Optional


class Job:

    DISTR_SCHEMES = {'uniform': 1}

    def __init__(self,
                 dataset_path: str,
                 batch_size: int,
                 epochs: int,
                 distr_scheme: str,
                 drop_last_batch: bool,
                 seed: Optional[int] = None,
                 config_path: Optional[str] = None,
                 libhdmlp_path: Optional[str] = None):
        libname = self._get_lib_path(libhdmlp_path)
        self.config_path = self._get_config_path(config_path)
        self.hdmlp_lib = ctypes.CDLL(libname)
        self.hdmlp_lib.get_next_file_end.restype = ctypes.c_ulonglong
        self.hdmlp_lib.get_staging_buffer.restype = ctypes.c_void_p
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
        self.job_id = None

    def _get_lib_path(self, configured_path) -> str:
        if configured_path is None:
            folder = pathlib.Path(__file__).parent.absolute() / "libhdmlp"
            library_name = "libhdmlp.so"
            if platform == "darwin":
                library_name = "libhdmlp.dylib"
            path = folder / library_name
        else:
            path = pathlib.Path(configured_path)
        if not path.exists():
            raise EnvironmentError("Couldn't find library at location {}".format(path))
        return str(path)

    def _get_config_path(self, configured_path) -> str:
        if configured_path is None:
            path = pathlib.Path(__file__).parent.absolute() / "data" / "hdmlp.cfg"
        else:
            path = pathlib.Path(configured_path)
        if not path.exists():
            raise EnvironmentError("Couldn't find configuration at location {}".format(path))
        return str(path)

    def setup(self):
        job_id = self.hdmlp_lib.setup(ctypes.c_wchar_p(self.dataset_path),
                                      ctypes.c_wchar_p(self.config_path),
                                      self.batch_size,
                                      self.epochs,
                                      self.distr_scheme,
                                      ctypes.c_bool(self.drop_last_batch),
                                      self.seed)
        buffer = self.hdmlp_lib.get_staging_buffer(job_id)
        self.job_id = job_id
        self.buffer_p = ctypes.cast(buffer, ctypes.POINTER(ctypes.c_char))

    def destroy(self):
        self.hdmlp_lib.destroy(self.job_id)

    def get(self):
        file_end = self.hdmlp_lib.get_next_file_end(self.job_id)
        if file_end < self.buffer_offset:
            self.buffer_offset = 0
        label_offset = 0
        while self.buffer_p[self.buffer_offset + label_offset] != b'\x00':
            label_offset += 1
        label = self.buffer_p[self.buffer_offset:self.buffer_offset + label_offset].decode('utf-8')
        file = self.buffer_p[self.buffer_offset + label_offset + 1:file_end]
        self.buffer_offset = file_end
        return label, file

    def length(self):
        return self.hdmlp_lib.length(self.job_id)

    def node_id(self):
        return self.hdmlp_lib.get_node_id(self.job_id)

    def no_nodes(self):
        return self.hdmlp_lib.get_no_nodes(self.job_id)
