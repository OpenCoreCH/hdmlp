from .hdmlpvision import HDMLPVisionDataset
from hdmlp import hdmlp

from PIL import Image

import os
import os.path
import io


def has_file_allowed_extension(filename, extensions):
    """Checks if a file is an allowed extension.

    Args:
        filename (string): path to a file
        extensions (tuple of strings): extensions to consider (lowercase)

    Returns:
        bool: True if the filename ends with one of given extensions
    """
    return filename.lower().endswith(extensions)


def is_image_file(filename):
    """Checks if a file is an allowed image extension.

    Args:
        filename (string): path to a file

    Returns:
        bool: True if the filename ends with a known image extension
    """
    return has_file_allowed_extension(filename, IMG_EXTENSIONS)

class HDMLPDatasetFolder(HDMLPVisionDataset):
    """A generic data loader where the samples are arranged in this way: ::

        root/class_x/xxx.ext
        root/class_x/xxy.ext
        root/class_x/xxz.ext

        root/class_y/123.ext
        root/class_y/nsdf3.ext
        root/class_y/asd932_.ext

    Args:
        root (string): Root directory path.
        hdmlp_job (hdmlp.Job): Configured HDMLP job for the dataset.
        transform (callable, optional): A function/transform that takes in
            a sample and returns a transformed version.
            E.g, ``transforms.RandomCrop`` for images.
        target_transform (callable, optional): A function/transform that takes
            in the target and transforms it.

     Attributes:
        classes (list): List of the class names.
        class_to_idx (dict): Dict with items (class_name, class_index).
        folder_to_idx (dict): Dict with items (folder_name, class_index)
        targets (list): The class_index value for each image in the dataset
    """

    def __init__(self, root, hdmlp_job: hdmlp.Job, transform=None,
                 target_transform=None):
        super(HDMLPDatasetFolder, self).__init__(root, transform=transform,
                                                 target_transform=target_transform)
        classes, class_to_idx = self._find_classes(self.root)

        self.job = hdmlp_job
        self.job.setup()

        self.classes = classes
        self.class_to_idx = class_to_idx

    def _find_classes(self, dir):
        """
        Finds the class folders in a dataset.

        Args:
            dir (string): Root directory path.

        Returns:
            tuple: (classes, class_to_idx) where classes are relative to (dir), and class_to_idx is a dictionary.

        Ensures:
            No class is a subdirectory of another.
        """
        classes = [d.name for d in os.scandir(dir) if d.is_dir()]
        classes.sort()
        class_to_idx = {classes[i]: i for i in range(len(classes))}
        return classes, class_to_idx

    def __getitem__(self, index):
        """
        Args:
            index (int): Index

        Returns:
            tuple: (sample, target) where target is class_index of the target class.
        """
        folder_label, sample = self.job.get()
        target = self.class_to_idx[folder_label]
        if self.transform is not None:
            sample = self.transform(sample)
        if self.target_transform is not None:
            target = self.target_transform(target)

        return sample, target

    def __len__(self):
        return self.job.length()

    def __del__(self):
        self.job.destroy()

    def get_job(self):
        return self.job

IMG_EXTENSIONS = ('.jpg', '.jpeg', '.png', '.ppm', '.bmp', '.pgm', '.tif', '.tiff', '.webp')


def pil_decode(object):
    img = Image.open(io.BytesIO(object))
    return img.convert('RGB')


class HDMLPImageFolder(HDMLPDatasetFolder):
    """A generic data loader where the images are arranged in this way: ::

        root/dog/xxx.png
        root/dog/xxy.png
        root/dog/xxz.png

        root/cat/123.png
        root/cat/nsdf3.png
        root/cat/asd932_.png

    Args:
        root (string): Root directory path.
        hdmlp_job (hdmlp.Job): Configured HDMLP job for the dataset.
        transform (callable, optional): A function/transform that  takes in an PIL image
            and returns a transformed version. E.g, ``transforms.RandomCrop``
        target_transform (callable, optional): A function/transform that takes in the
            target and transforms it.

     Attributes:
        classes (list): List of the class names.
        class_to_idx (dict): Dict with items (class_name, class_index).
    """

    def __init__(self, root, hdmlp_job: hdmlp.Job, transform=None, target_transform=None):
        super(HDMLPImageFolder, self).__init__(root, hdmlp_job,
                                               transform=transform,
                                               target_transform=target_transform)


    def __getitem__(self, item):
        folder_label, raw_sample = self.job.get()
        sample = pil_decode(raw_sample)
        target = self.class_to_idx[folder_label]
        if self.transform is not None:
            sample = self.transform(sample)
        if self.target_transform is not None:
            target = self.target_transform(target)

        return sample, target