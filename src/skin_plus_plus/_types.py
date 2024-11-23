from __future__ import annotations

import typing
from typing import Callable
from typing import Sequence
from typing import TypedDict
from typing import Union

import numpy as np

from . import SkinData

try:
    import maya.cmds as _  # noqa: F401
except (ImportError, ModuleNotFoundError):
    from pymxs import runtime as mxrt

T_Node = Union[mxrt.Node, str]
"""Generic node type"""

T_Handle = Union[int, str]
"""Unique handle for identifying a node in a dcc scene"""

T_Float64Array = np.ndarray[typing.Any, np.dtype[np.float64]]
"""Array for holding skin weights"""

T_Int32Array = np.ndarray[typing.Any, np.dtype[np.int32]]
"""Array for holding bone IDs"""

C_ApplySkinData = Callable[[T_Handle, SkinData], bool]
"""Callable signature for `apply_skin_data`"""

C_ExtractSkinData = Callable[[T_Handle, Union[Sequence[int], None]], SkinData]
"""Callable signature for `extract_skin_data`"""

C_GetVertexPositons = Callable[[T_Handle], T_Float64Array]
"""Callable signature for `get_vertex_weights`"""


class T_JsonSkinData(TypedDict):
    bone_names: list[str]
    bone_ids: list[list[int]]
    weights: list[list[float]]
    positions: list[list[float]]
