from __future__ import annotations

import enum


class ApplicationMode(enum.Enum):
    order = 0
    nearest = 1
    # Disable until implemented:
    # nearest_n = 2
    # barycentric = 3


class FileType(enum.Enum):
    """
    Enum to specify the type of file to save skin data as.

    Arguments:
    ----------

    - `json`
    - `pickle`
    """

    json = 0
    pickle = 1
