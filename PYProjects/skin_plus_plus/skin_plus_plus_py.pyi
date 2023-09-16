import numpy as np
import numpy.typing as np_typing
import typing


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
