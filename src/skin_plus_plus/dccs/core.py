from __future__ import annotations

import abc
import importlib
import numpy as np
import pathlib
import scipy.spatial as spatial
import skin_plus_plus
import typeguard
import typing

from .. import SkinData
from .. import enums

_typing = False
if _typing:
    from .. import _types
    from typing import Sequence

T_HostNode = typing.TypeVar("T_HostNode")


class IHost(typing.Generic[T_HostNode], metaclass=abc.ABCMeta):
    if _typing:
        _extract_skin_data: _types.C_ExtractSkinData
        _apply_skin_data: _types.C_ApplySkinData
        _get_vertex_positions: _types.C_GetVertexPositons

    def __init__(self) -> None:
        self._get_dcc_backend()

    @property
    @abc.abstractmethod
    def name(self) -> str:
        """
        The name of the host
        """

    @property
    @abc.abstractmethod
    def api_name(self) -> str:
        """
        The api name of the compiled backend for the host.
        i.e. pymxs or pymaya
        """

    def _get_correct_bone_names(self, skin_data_bone_names: list[str], namespace: str) -> list[str]:
        namespace = namespace if namespace.endswith(":") else f"{namespace}:"
        clean_names = (name.split(":")[-1] for name in skin_data_bone_names)
        namespaced_names = [f"{namespace}{name}" for name in clean_names]
        return namespaced_names
        # namespace_map = {
        #     full_name.replace(namespace, ""): full_name
        #     for full_name in skin_data_bone_names
        # }
        # bone_names = [name.split(":")[-1] for name in skin_data_bone_names]
        # name_map = {name: index for index, name in enumerate(bone_names)}
        # for short_name, full_name in namespace_map.items():
        #     if short_name not in name_map:
        #         continue

        #     bone_names[name_map[short_name]] = full_name

        # return bone_names

    def _get_dcc_backend(self):
        version = self._get_version_number()
        current_directory = pathlib.Path(__file__).parent
        sub_module_path = current_directory / self.name / str(version)

        if not sub_module_path.exists():
            raise FileNotFoundError(f"Unsupported DCC version: {version}")

        import_path = (
            f"{__name__.rstrip('core')}{self.name}.{version}.skin_plus_plus_{self.api_name}"
        )
        backend = importlib.import_module(import_path)

        self._extract_skin_data = backend.extract_skin_data
        self._apply_skin_data = backend.apply_skin_data
        self._get_vertex_positions = backend.get_vertex_positions

        return backend

    @abc.abstractmethod
    def _get_version_number(self) -> int | str:
        """
        Get the version number of the host
        """

    @abc.abstractmethod
    def get_current_file_path(self) -> pathlib.Path:
        """
        Get the file path of the current host scene
        """

    @abc.abstractmethod
    def get_selection(self) -> tuple[T_HostNode, ...]:
        """
        Get the selection of the current host scene
        """

    @abc.abstractmethod
    def get_node_name(self, node: T_HostNode) -> str:
        """
        Get the name of the given node
        """

    @abc.abstractmethod
    def get_vertex_positions(self, node: T_HostNode) -> _types.T_Float64Array:
        """
        Get the vertex postions of the given node
        """

    @abc.abstractmethod
    def get_node_handle(self, node: T_HostNode) -> _types.T_Handle:
        """
        Get the unique handle of the given node
        """

    def extract_skin_data(
        self,
        node: T_HostNode,
        bone_names: Sequence[str] | None = None,
        normalize: bool = False,
        vertex_ids: Sequence[int] | None = None,
    ) -> SkinData:
        if bone_names:
            return self._extract_skin_data_from_bones(
                node, bone_names, normalize=normalize, vertex_ids=vertex_ids
            )

        handle = self.get_node_handle(node)
        return self._extract_skin_data(handle, vertex_ids)

    def _extract_skin_data_from_bones(
        self,
        node: T_HostNode,
        bone_names: Sequence[str],
        normalize: bool = False,
        vertex_ids: Sequence[int] | None = None,
    ):
        skin_data = skin_plus_plus.extract_skin_data(node, vertex_ids=vertex_ids)
        bone_names_map = {
            index: bone_name
            for index, bone_name in enumerate(skin_data.bone_names)
            if bone_name in bone_names
        }

        vertex_count = len(skin_data.weights)
        final_weights = np.zeros((vertex_count, 8), dtype=np.float64)
        final_bone_ids = np.full((vertex_count, 8), -1, dtype=np.int32)
        final_positions = np.zeros((vertex_count, 3), dtype=np.float64)
        for vertex_index, bone_ids in enumerate(skin_data.bone_ids):
            new_weights = np.zeros(8, dtype=np.float64)
            new_bone_ids = np.full(8, -1, dtype=np.int32)
            next_influence_index = 0
            for new_index, bone_id in enumerate(bone_names_map):
                influence_indexes = np.where(bone_ids == bone_id)[0]
                if not influence_indexes.size:
                    continue

                for influence_index in influence_indexes:
                    weights = skin_data.weights[vertex_index]
                    new_weights[next_influence_index] = weights[influence_index]
                    new_bone_ids[next_influence_index] = new_index
                    next_influence_index += 1

                final_weights[vertex_index] = new_weights
                final_bone_ids[vertex_index] = new_bone_ids
                final_positions[vertex_index] = skin_data.positions[vertex_index]

        if normalize:
            for index, data in enumerate(final_weights):
                if not (norm := np.linalg.norm(data, 1)):
                    continue

                final_weights[index] = data / norm

        return skin_plus_plus.SkinData(
            list(bone_names_map.values()), final_bone_ids, final_weights, final_positions
        )

    def apply_skin_data(
        self,
        node: T_HostNode,
        skin_data: SkinData | Sequence[SkinData],
        namespace: str | None = None,
        application_mode: enums.ApplicationMode = enums.ApplicationMode.order,
    ) -> bool:
        if isinstance(skin_data, SkinData):
            return self._apply_skin_data_(
                node, skin_data, namespace=namespace, application_mode=application_mode
            )

        return self._apply_skin_datas_(node, skin_data, namespace=namespace)

    def _apply_skin_data_(
        self,
        node: T_HostNode,
        skin_data: SkinData,
        namespace: str | None = None,
        application_mode: enums.ApplicationMode = enums.ApplicationMode.order,
    ):
        if application_mode == enums.ApplicationMode.nearest:
            current_positions = self.get_vertex_positions(node)
            kd_tree = spatial.KDTree(skin_data.positions)
            _, new_indexes = kd_tree.query(current_positions)
            index_map = typeguard.check_type(
                {old_index: new_index for old_index, new_index in enumerate(new_indexes)},
                dict[int, np.int64],
            )
            new_weights = np.array(
                tuple(skin_data.weights[index_map[index]] for index in index_map)
            )
            new_bone_ids = np.array(
                tuple(skin_data.bone_ids[index_map[index]] for index in index_map)
            )
            bone_names = (
                self._get_correct_bone_names(skin_data.bone_names, namespace)
                if namespace
                else skin_data.bone_names
            )
            new_skin_data = skin_plus_plus.SkinData(
                bone_names,
                new_bone_ids,
                new_weights,
                skin_data.positions,
            )
            skin_data = new_skin_data

        elif namespace:
            bone_names = self._get_correct_bone_names(skin_data.bone_names, namespace)
            skin_data = skin_plus_plus.SkinData(
                bone_names,
                skin_data.bone_ids,
                skin_data.weights,
                skin_data.positions,
            )

        handle = self.get_node_handle(node)
        return self._apply_skin_data(handle, skin_data)

    def _apply_skin_datas_(
        self,
        node: T_HostNode,
        skin_datas: Sequence[SkinData],
        namespace: str | None = None,
    ) -> bool:
        handle = self.get_node_handle(node)

        def _set_weights_shape_to_size(weights: np.ndarray, size: int):
            if weights.shape[1] < size:
                weights = weights.copy()
                zeros = np.zeros((weights.shape[0], (size - weights.shape[1])), dtype=np.float64)
                weights = np.append(weights, zeros, axis=1)

            return weights

        def _set_bone_ids_shape_to_size(bone_ids: np.ndarray, size: int):
            if bone_ids.shape[1] < size:
                bone_ids = bone_ids.copy()
                zeros = np.full((bone_ids.shape[0], (size - bone_ids.shape[1])), -1, dtype=np.int64)
                bone_ids = np.append(bone_ids, zeros, axis=1)

            return bone_ids

        max_influence_count = max(skin_data.weights.shape[1] for skin_data in skin_datas)
        bone_name_maps: list[dict[int, str]] = []
        combined_bone_names = skin_datas[0].bone_names.copy()
        for skin_data in skin_datas:
            skin_data.weights = _set_weights_shape_to_size(skin_data.weights, max_influence_count)
            skin_data.bone_ids = _set_bone_ids_shape_to_size(
                skin_data.bone_ids, max_influence_count
            )
            bone_name_maps.append({index: name for index, name in enumerate(skin_data.bone_names)})
            combined_bone_names.extend(skin_data.bone_names)

        assert combined_bone_names, f"combined_bone_names: {combined_bone_names}"
        combined_bone_names = list(dict.fromkeys(combined_bone_names))
        combined_bone_name_map = {name: index for index, name in enumerate(combined_bone_names)}

        current_positions = self.get_vertex_positions(node)

        temp_skin_datas = list(skin_datas)
        first_skin_data = temp_skin_datas.pop(0)
        combined_positions = first_skin_data.positions.copy()
        combined_skin_weights = first_skin_data.weights.copy()
        combined_bone_ids = first_skin_data.bone_ids.copy()
        for skin_data in temp_skin_datas:
            combined_positions = np.append(combined_positions, skin_data.positions, axis=0)
            combined_skin_weights = np.append(combined_skin_weights, skin_data.weights, axis=0)
            combined_bone_ids = np.append(combined_bone_ids, skin_data.bone_ids, axis=0)

        kd_tree = spatial.KDTree(combined_positions)
        _, combined_vertex_indexes = kd_tree.query(current_positions)
        vertex_index_map = typeguard.check_type(
            {old_index: new_index for old_index, new_index in enumerate(combined_vertex_indexes)},
            dict[int, np.int64],
        )

        sorted_weights = np.array(
            tuple(combined_skin_weights[vertex_index_map[index]] for index in vertex_index_map),
            dtype=np.float64,
        )
        sorted_bone_ids = np.array(
            tuple(combined_bone_ids[vertex_index_map[index]] for index in vertex_index_map),
            dtype=np.int32,
        )
        skin_data_ranges = []
        start = 0
        end = 0
        for skin_data in skin_datas:
            vertex_count = len(skin_data.weights)
            end += vertex_count
            skin_data_ranges.append((start, end))
            start += vertex_count

        for index, sorted_index in vertex_index_map.items():
            skin_data_index = None
            for _skin_data_index, skin_data_range in enumerate(skin_data_ranges):
                if skin_data_range[0] <= sorted_index < skin_data_range[1]:
                    skin_data_index = _skin_data_index
                    break

            assert skin_data_index is not None, (
                f"index: {index}, skin_data_range: {skin_data_ranges}"
            )
            bone_ids = sorted_bone_ids[index].copy()
            bone_name_map = bone_name_maps[skin_data_index]
            for bone_id_index, bone_id in enumerate(bone_ids):
                if bone_id == -1:
                    break

                bone_name = bone_name_map[bone_id]
                correct_id = combined_bone_name_map[bone_name]
                bone_ids[bone_id_index] = correct_id

            sorted_bone_ids[index] = bone_ids

        combined_bone_names = (
            self._get_correct_bone_names(combined_bone_names, namespace)
            if namespace
            else combined_bone_names
        )
        combined_skin_data = skin_plus_plus.SkinData(
            combined_bone_names, sorted_bone_ids, sorted_weights, current_positions
        )
        return self._apply_skin_data(handle, combined_skin_data)
