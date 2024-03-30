from __future__ import annotations

import pathlib
import pymel.core as pm_core
import pymel.core.nodetypes as pm_ntypes

from .. import core
from pymel import versions


class IHost(core.IHost):

    @property
    def name(self) -> str:
        return "maya"

    @property
    def api_name(self) -> str:
        return "pymaya"

    def _get_version_number(self) -> str:
        version_number = str(versions.current())[:4]
        return version_number

    def get_current_file_path(self) -> pathlib.Path:
        scene_name = pm_core.sceneName()
        if not scene_name:
            raise RuntimeError("File is not saved!")

        return pathlib.Path(scene_name)

    def get_selection(self) -> tuple[pm_ntypes.DagNode, ...]:
        return tuple(pm_core.ls(selection=True))

    def get_node_name(self, node: pm_ntypes.DagNode) -> str:
        return node.name(stripNamespace=True)

    def get_node_handle(self, node: pm_ntypes.DagNode) -> str:
        return node.longName()
