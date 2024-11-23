from __future__ import annotations

import typing

from . import _types

class SkinData:
    """
    Class containing data for a given skin object.

    ---

    This class is a wrapped c++ struct exposed to Python with Pybind11.

    # It cannot be extended!

    ---

    Attributes:
    -----------
    - `bone_names`: The names of the bones in the skin object.
    - `bone_ids`: The ids of each influence assigned to each vertext.
        These are used to map to the bone names.
    - `weights`: The weights of each influence assigned to each vertext.
    - `positions`: The positions of each vertex.
    - `vertex_ids`: The specific vertex ids that make up the SkinData.
        If `None` then all vertices are used to make up the SkinData.

    ---

    ```python
    SkinData()
    SkinData(vertex_ids: numpy.ndarray[numpy.int32[1, n]], max_influence_count: int)
    SkinData(vertex_count: int, max_influence_count: int)
    SkinData(skin_data: skin_plus_plus.py.310.skin_plus_plus_py.SkinData)
    SkinData(skin_bone_names: List[str], vertex_bone_ids: numpy.ndarray[numpy.int32[m, n]], vertex_weights: numpy.ndarray[numpy.float64[m, n]], vertex_positions: numpy.ndarray[numpy.float64[m, n]], vertex_ids: Optional[numpy.ndarray[numpy.int32[1, n]]] = None)
    SkinData(skin_tuple: tuple)
    ```
    """

    bone_names: list[str]
    """The names of the bones in the SkinData"""
    bone_ids: _types.T_Int32Array
    """The bone ids for each influence on each vertex"""
    weights: _types.T_Float64Array
    """The weights for each influence on each vertex"""
    positions: _types.T_Float64Array
    """The position of each vertex in the SkinData's mesh"""
    vertex_ids: _types.T_Int32Array | None = None
    """
    The specific vertex ids that make up the SkinData.
    If `None` then all vertices are used to make up the SkinData.
    """

    @typing.overload
    def __init__(self) -> None:
        ...

    @typing.overload
    def __init__(
        self,
        names: tuple[str, ...],
        bone_ids: tuple[tuple[int, ...], ...],
        weights: tuple[tuple[float, ...], ...],
        positions: tuple[tuple[float, float, float], ...],
    ) -> None:
        ...

    @typing.overload
    def __init__(
        self,
        names: tuple[str, ...],
        bone_ids: tuple[list[int], ...],
        weights: tuple[list[float], ...],
        positions: tuple[list[float], ...],
    ) -> None:
        ...

    @typing.overload
    def __init__(
        self,
        skin_bone_names: list[str],
        vertex_bone_ids: _types.T_Int32Array,
        vertex_weights: _types.T_Float64Array,
        vertex_positions: _types.T_Float64Array,
        vertex_ids: _types.T_Int32Array | None = None,
    ) -> None:
        ...
