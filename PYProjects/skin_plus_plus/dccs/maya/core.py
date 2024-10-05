from __future__ import annotations

import maya.api.OpenMaya as om
import numpy as np
import numpy.dtypes as np_dtypes
import pathlib
import pymel.core as pm_core
import pymel.core.nodetypes as pm_ntypes

from .. import core
from pymel import versions


class IHost(core.IHost[pm_ntypes.DagNode]):
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

    def get_node_name(self, node) -> str:
        return node.name(stripNamespace=True)

    def get_vertex_positions(self, node) -> np.ndarray:
        node_name = node.name()
        selection_list = om.MGlobal.getSelectionListByName(node_name)
        dag_path = selection_list.getDagPath(0)
        fn_mesh = om.MFnMesh(dag_path)
        num_vertices = fn_mesh.numVertices
        vertex_positions = np.zeros([num_vertices, 3])
        points = fn_mesh.getPoints(space=om.MSpace.kWorld)
        for index, point in enumerate(points):
            vertex_positions[index] = np.array(
                (point.x, point.y, point.z), dtype=np_dtypes.Float64DType
            )

        return vertex_positions

    def get_node_handle(self, node) -> str:
        return node.longName()
