from __future__ import annotations

import pathlib

from pymxs import runtime as mxrt

from .. import core


class IHost(core.IHost):

    @property
    def name(self) -> str:
        return "max"

    @property
    def api_name(self) -> str:
        return "pymxs"

    def _get_version_number(self):
        version_info = mxrt.MaxVersion()
        version_number = version_info[7]
        return version_number

    def get_current_file_path(self) -> pathlib.Path:
        max_file_path = mxrt.MaxFilePath
        if not max_file_path:
            raise RuntimeError("File is not saved!")

        return pathlib.Path(max_file_path, mxrt.MaxFileName)

    def get_selection(self) -> tuple[mxrt.Node, ...]:
        return tuple(mxrt.Selection)

    def get_node_name(self, node: mxrt.Node) -> str:
        return node.Name

    def get_node_handle(self, node: mxrt.Node) -> int:
        return node.Handle
