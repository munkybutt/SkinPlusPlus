from __future__ import annotations

import pathlib

from pymxs import runtime as mxrt
from typing_extensions import override

from .. import core

_typing = False
if _typing:
    from ... import _types
del _typing


class IHost(core.IHost[mxrt.Node]):
    @property
    @override
    def name(self) -> str:
        return "max"

    @property
    @override
    def api_name(self) -> str:
        return "pymxs"

    @override
    def _get_version_number(self):
        version_info = mxrt.MaxVersion()
        version_number = version_info[7]
        return version_number

    @override
    def get_current_file_path(self) -> pathlib.Path:
        max_file_path = mxrt.MaxFilePath
        if not max_file_path:
            raise RuntimeError("File is not saved!")

        return pathlib.Path(max_file_path, mxrt.MaxFileName)

    @override
    def get_selection(self) -> tuple[mxrt.Node, ...]:
        return tuple(mxrt.Selection)

    @override
    def get_node_name(self, node) -> str:
        return node.Name

    @override
    def get_vertex_positions(self, node) -> _types.T_Float64Array:
        return self._get_vertex_positions(node.Name)

    @override
    def get_node_handle(self, node) -> int:
        return node.Handle
