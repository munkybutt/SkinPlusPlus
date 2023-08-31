import numpy as np
import numpy.typing as np_typing
import typing

from .core import FileType as _FileType
from .core import export_skin_data as _export_skin_data
from .core import import_skin_data as _import_skin_data
from .io import save as _save
from .io import load as _load
from .io import max_to_maya as _max_to_maya
from .io import maya_to_max as _maya_to_max


FileType = _FileType
export_skin_data = _export_skin_data
import_skin_data = _import_skin_data
save = _save
load = _load
max_to_maya = _max_to_maya
maya_to_max = _maya_to_max


current_dcc: str = ...
"""
The name of the current DCC.
"""


class SkinData:
    """
    Class containing data for a given skin object.

    ---

    This class is a wrapped c++ struct exposed to Python with Pybind11.

    # Note: It cannot be extended!

    ---

    Attributes:
    -----------
    - `bone_names`: The names of the bones in the skin object.
    - `bone_ids`: The ids of each influence assigned to each vertext.
        These are used to map to the bone names.
    - `weights`: The weights of each influence assigned to each vertext.
    - `positions`: The positions of each vertex.
    """

    bone_names: list[str]
    bone_ids: np_typing.NDArray[np.int64]
    weights: np_typing.NDArray[np.float32]
    positions: np_typing.NDArray[np.float32]

    @typing.overload
    def __init__(self):
        ...

    @typing.overload
    def __init__(
        self,
        names: tuple[str, ...],
        bone_ids: tuple[tuple[int, ...], ...],
        weights: tuple[tuple[float, ...], ...],
        positions: tuple[tuple[float, float, float], ...],
    ):
        ...


def get_skin_data(mesh_name: str) -> SkinData:
    ...


def set_skin_weights(mesh_name: str, skin_data: SkinData) -> bool:
    ...
