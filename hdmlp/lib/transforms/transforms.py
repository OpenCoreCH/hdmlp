import ctypes


class Transform:
    UNKNOWN_DIMENSION = -1
    UNKNOWN_SIZE = -1

    arg_types = []

    def get_output_dimensions(self, w_in, h_in):
        """
        Returns the dimensions of the image after applying the transformations
        :param w_in:
        :param h_in:
        :return:
        """
        return (self.UNKNOWN_DIMENSION, self.UNKNOWN_DIMENSION)

    def get_output_size(self, w_in, h_in):
        """
        Returns the output size (in byte) of the transform
        :param w_in: width of the input image
        :param h_in: height of the input image
        """
        return self.UNKNOWN_SIZE

    def get_args(self):
        """
        Returns the arguments of the transformation as a (flattened) list
        """
        return []


class ImgDecode(Transform):
    pass

class CVImageManipulation(Transform):

    def get_output_size(self, w_in, h_in):
        (dim_x, dim_y) = self.get_output_dimensions(w_in, h_in)
        return dim_x * dim_y * 3

class Resize(CVImageManipulation):
    arg_types = [ctypes.c_int, ctypes.c_int]

    def __init__(self, w_out, h_out):
        self.w = w_out
        self.h = h_out

    def get_output_dimensions(self, w_in, h_in):
        return (self.w, self.h)

    def get_args(self):
        return [self.w, self.h]

class CenterCrop(CVImageManipulation):
    arg_types = [ctypes.c_int, ctypes.c_int]

    def __init__(self, w_out, h_out):
        self.w = w_out
        self.h = h_out

    def get_output_dimensions(self, w_in, h_in):
        return (self.w, self.h)

    def get_args(self):
        return [self.w, self.h]

class RandomHorizontalFlip(CVImageManipulation):
    arg_types = [ctypes.c_float]

    def __init__(self, p = 0.5):
        self.p = p

    def get_output_dimensions(self, w_in, h_in):
        return (w_in, h_in)

    def get_args(self):
        return [self.p]


class RandomVerticalFlip(CVImageManipulation):
    arg_types = [ctypes.c_float]

    def __init__(self, p=0.5):
        self.p = p

    def get_output_dimensions(self, w_in, h_in):
        return (w_in, h_in)

    def get_args(self):
        return [self.p]


class RandomResizedCrop(CVImageManipulation):
    arg_types = [ctypes.c_int, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float]

    def __init__(self, size, scale=(0.08, 1.0), ratio=(3. / 4., 4. / 3.)):
        self.size = size
        self.scale = scale
        self.ratio = ratio

    def get_output_dimensions(self, w_in, h_in):
        return (self.size, self.size)

    def get_args(self):
        return [self.size, self.scale[0], self.scale[1], self.ratio[0], self.ratio[1]]

class ToTensor(Transform):

    def __init__(self):
        pass

    def get_output_dimensions(self, w_in, h_in):
        return (w_in, h_in)

    def get_output_size(self, w_in, h_in):
        return w_in * h_in * 3 * 4  # 3 Channel FP32 Tensor


class Normalize(Transform):
    arg_types = [ctypes.c_double * 3, ctypes.c_double * 3]

    def __init__(self, mean, std):
        self.mean = mean
        self.std = std

    def get_output_dimensions(self, w_in, h_in):
        return (w_in, h_in)

    def get_output_size(self, w_in, h_in):
        return w_in * h_in * 3 * 4  # 3 Channel FP32 Tensor

    def get_args(self):
        return [(ctypes.c_double * 3)(*self.mean), (ctypes.c_double * 3)(*self.std)]
