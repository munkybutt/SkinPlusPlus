from __future__ import annotations

import pathlib

import bpy
import bpy.types as bpy_types
from typing_extensions import override

from .. import core

_typing = False
if _typing:
    from collections.abc import Sequence

    from ... import _types
del _typing


class IHost(core.IHost[bpy_types.Object]):
    @property
    @override
    def name(self) -> str:
        return "blender"

    @property
    @override
    def api_name(self) -> str:
        return "bpy"

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
    def get_selection(self) -> tuple[bpy_types.Object, ...]:
        return tuple(mxrt.Selection)

    @override
    def get_node_name(self, node: bpy_types.Object) -> str:
        return node.name

    @override
    def get_vertex_positions(self, node: bpy_types.Object) -> _types.T_Float64Array:
        return self._get_vertex_positions(node.Name)

    @override
    def get_node_handle(self, node: bpy_types.Object) -> int:
        return node.Handle

    @override
    def extract_skin_data(
        self, node: bpy_types.Object, vertex_ids: Sequence[int] | None = None
    ) -> core.SkinData:
        armature = self._get_armature(node)
        if armature is None:
            raise RuntimeError(f"No armature found for node: {node}")

        vertices = node.data.vertices
        vertex_group_names = [vertex_group.name for vertex_group in node.vertex_groups]
        for bone in armature.pose.bones:
            if bone.name not in vertex_group_names:
                continue

            bone_index = node.vertex_groups[bone.name].index
            bone_vertices = (
                vertex
                for vertex in vertices
                if bone_index in {group.group for group in vertex.groups}
            )

            for bone_vertex in bone_vertices:
                weight = bone_vertex.groups[bone_index].weight
                print(f"Vertex: {bone_vertex.index} weight: {weight} bone: {bone.name}")

    def _get_armature(self, node: bpy_types.Object) -> bpy_types.Object | None:
        parent = node.parent
        while parent:
            if parent.type == "ARMATURE":
                return parent






def _get_armature(node: bpy_types.Object) -> bpy_types.Object | None:
    parent = node.parent
    while parent:
        if parent.type == "ARMATURE":
            return parent

def extract_skin_data(
    node: bpy_types.Object, vertex_ids: Sequence[int] | None = None
):
    armature = _get_armature(node)
    if armature is None:
        raise RuntimeError(f"No armature found for node: {node}")

    vertices = node.data.vertices
    skin_data = [[0.0, 0.0]] * len(vertices)
    vertex_group_names = [vertex_group.name for vertex_group in node.vertex_groups]
    for bone in armature.pose.bones:
        if (bone_name := bone.name) not in vertex_group_names:
            continue

        # bone_index = node.vertex_groups[bone_name].index
        # bone_vertices = (
        #     vertex
        #     for vertex in vertices
        #     if bone_index in {group.group for group in vertex.groups}
        # )
        for vertex in vertices:
            for influence in vertex.groups:
                skin_data[vertex.index][influence.group] = influence.weight

        # for bone_vertex in bone_vertices:
        #     weight = bone_vertex.groups[bone_index].weight
        #     # print(f"Vertex: {bone_vertex.index} weight: {weight} bone: {bone.name}")
        #     skin_data[bone_vertex.index][bone_index] = weight
        #     print(f"[{bone_vertex.index}][{bone_index}] = {weight}")

    for weights in skin_data:
        print(weights)


