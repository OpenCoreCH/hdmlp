import ctypes
import pathlib
from sys import platform
from typing import Optional, List
from .lib.transforms import transforms
import numpy as np


class Job:

    DISTR_SCHEMES = {'uniform': 1}

    def __init__(self,
                 dataset_path: str,
                 batch_size: int,
                 epochs: int,
                 distr_scheme: str,
                 drop_last_batch: bool,
                 transforms: Optional[List[transforms.Transform]] = None,
                 seed: Optional[int] = None,
                 config_path: Optional[str] = None,
                 libhdmlp_path: Optional[str] = None,
                 filesystem_backend: Optional[str] = "filesystem"):
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
        self.transforms = [] if transforms is None else transforms
        self.transformed_size = 0
        self.trans_w, self.trans_h = None, None
        if transforms is not None:
            self._get_transformed_size()
        self.seed = seed
        self.filesystem_backend = filesystem_backend
        self.buffer_p = None
        self.buffer_offset = 0
        self.job_id = None

    def _get_lib_path(self, configured_path) -> str:
        if configured_path is None:
            folder = pathlib.Path(__file__).parent.parent.absolute()
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

    def _get_transformed_size(self):
        w, h = self.get_transformed_dims()
        out_size = self.transforms[-1].get_output_size(w, h)
        if out_size == transforms.Transform.UNKNOWN_SIZE:
            raise ValueError("Can't determine the output size after applying the transformations")
        self.transformed_size = out_size

    def get_transformed_dims(self):
        if self.trans_w is None or self.trans_h is None:
            w, h = transforms.Transform.UNKNOWN_DIMENSION, transforms.Transform.UNKNOWN_DIMENSION
            for transform in self.transforms:
                w, h = transform.get_output_dimensions(w, h)
            self.trans_w, self.trans_h = w, h
        return self.trans_w, self.trans_h

    def setup(self):
        cpp_transform_names = [transform.__class__.__name__ for transform in self.transforms]
        cpp_transform_names_arr = (ctypes.c_wchar_p * len(cpp_transform_names))()
        cpp_transform_names_arr[:] = cpp_transform_names
        transform_arg_size = sum(sum(ctypes.sizeof(arg) for arg in transform.arg_types) for transform in self.transforms)
        transform_args_arr = (ctypes.c_byte * transform_arg_size)()
        transform_args_arr_p = ctypes.cast(ctypes.pointer(transform_args_arr), ctypes.c_void_p)
        for transform in self.transforms:
            arg_types = transform.arg_types
            args = transform.get_args()
            for type, arg in zip(arg_types, args):
                p = ctypes.cast(transform_args_arr_p, ctypes.POINTER(type))
                p[0] = arg
                transform_args_arr_p.value += ctypes.sizeof(type)
        job_id = self.hdmlp_lib.setup(ctypes.c_wchar_p(self.dataset_path),
                                      ctypes.c_wchar_p(self.config_path),
                                      self.batch_size,
                                      self.epochs,
                                      self.distr_scheme,
                                      ctypes.c_bool(self.drop_last_batch),
                                      self.seed,
                                      cpp_transform_names_arr,
                                      transform_args_arr,
                                      self.transformed_size,
                                      len(cpp_transform_names),
                                      ctypes.c_wchar_p(self.filesystem_backend))
        buffer = self.hdmlp_lib.get_staging_buffer(job_id)
        self.job_id = job_id
        self.buffer_p = ctypes.cast(buffer, ctypes.POINTER(ctypes.c_char))

    def destroy(self):
        self.hdmlp_lib.destroy(self.job_id)

    def get(self, num_items = 1, decode_as_np_array=False, np_array_shape=None):
        labels = []
        file_end = self.hdmlp_lib.get_next_file_end(self.job_id)
        if file_end < self.buffer_offset:
            self.buffer_offset = 0
        label_offset = 0
        self.label_distance = 0
        for i in range(num_items):
            prev_label_offset = label_offset
            while self.buffer_p[self.buffer_offset + label_offset] != b'\x00':
                label_offset += 1
            labels.append(self.buffer_p[self.buffer_offset + prev_label_offset:self.buffer_offset + label_offset].decode('utf-8'))
            if num_items > 1:
                if i == 0 and self.label_distance == 0:
                    while self.buffer_p[self.buffer_offset + label_offset] == b'\x00':
                        label_offset += 1
                    self.label_distance = label_offset
                    #print(label_distance)
                else:
                    label_offset = prev_label_offset + self.label_distance
            else:
                label_offset += 1
        if decode_as_np_array:
            file = np.ctypeslib.as_array(ctypes.cast(ctypes.cast(self.buffer_p, ctypes.c_void_p).value + self.buffer_offset + label_offset, ctypes.POINTER(ctypes.c_float)),
                                         np_array_shape)
        else:
            file = self.buffer_p[self.buffer_offset + label_offset:file_end]
        self.buffer_offset = file_end
        if num_items == 1:
            labels = labels[0]
        return labels, file

    def length(self):
        return self.hdmlp_lib.length(self.job_id)

    def get_node_id(self):
        return self.hdmlp_lib.get_node_id(self.job_id)

    def get_no_nodes(self):
        return self.hdmlp_lib.get_no_nodes(self.job_id)

    def get_batch_size(self):
        return self.batch_size

    def get_num_epochs(self):
        return self.epochs

    def get_drop_last_batch(self):
        return self.drop_last_batch

    def get_transforms(self):
        return self.transforms

    def get_metrics(self):
        metrics = {
            "stall_time": [],
            "augmentation_time": [],
            "read_times": [],
            "read_locations": []
        }
        stall_time_size = self.hdmlp_lib.get_metric_size(self.job_id, "stall_time", 0, 0)
        if stall_time_size == 0:
            print("No metrics acquired during run, did you set HDMLPPROFILING to 1?")
        else:
            self.hdmlp_lib.get_stall_time.restype = ctypes.POINTER(ctypes.c_double * stall_time_size)
            metrics["stall_time"] = [e for e in self.hdmlp_lib.get_stall_time(self.job_id).contents]
            prefetcher_threads = self.hdmlp_lib.get_metric_size(self.job_id, "augmentation_time", 0, 0)
            for i in range(prefetcher_threads):
                num_elems = self.hdmlp_lib.get_metric_size(self.job_id, "augmentation_time_thread", i, 0)
                self.hdmlp_lib.get_augmentation_time.restype = ctypes.POINTER(ctypes.c_double * num_elems)
                metrics["augmentation_time"].append([e for e in self.hdmlp_lib.get_augmentation_time(self.job_id, i).contents])
            storage_classes = self.hdmlp_lib.get_metric_size(self.job_id, "read_times", 0, 0)
            for i in range(storage_classes):
                class_read_times = []
                class_read_locations = []
                num_threads = self.hdmlp_lib.get_metric_size(self.job_id, "read_times_threads", i, 0)
                for j in range(num_threads):
                    num_elems = self.hdmlp_lib.get_metric_size(self.job_id, "read_times_threads_elem", i, j)
                    self.hdmlp_lib.get_read_times.restype = ctypes.POINTER(ctypes.c_double * num_elems)
                    self.hdmlp_lib.get_read_locations.restype = ctypes.POINTER(ctypes.c_int * num_elems)
                    if num_elems == 0:
                        class_read_times.append([])
                        class_read_locations.append([])
                    else:
                        class_read_times.append([e for e in self.hdmlp_lib.get_read_times(self.job_id, i, j).contents])
                        class_read_locations.append([e for e in self.hdmlp_lib.get_read_locations(self.job_id, i, j).contents])
                metrics["read_times"].append(class_read_times)
                metrics["read_locations"].append(class_read_locations)
        return metrics