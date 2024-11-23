from __future__ import annotations

import abc
import importlib
import pathlib
import typing

import numpy as np
import scipy.spatial as spatial
import typeguard

import skin_plus_plus

from .. import enums

_typing = False
if _typing:
    from typing import Sequence

    from .. import SkinData
    from .. import _types

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

    def _get_dcc_backend(self):
        version = self._get_version_number()
        current_directory = pathlib.Path(__file__).parent
        sub_module_path = current_directory / self.name / str(version)

        if not sub_module_path.exists():
            raise FileNotFoundError(f"Unsupported DCC version: {version}")

        import_path = f"{__name__.rstrip('core')}{self.name}.{version}.skin_plus_plus_{self.api_name}"
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
        self, node: T_HostNode, vertex_ids: Sequence[int] | None = None
    ) -> SkinData:
        handle = self.get_node_handle(node)
        return self._extract_skin_data(handle, vertex_ids=vertex_ids)

    def apply_skin_data(
        self,
        node: T_HostNode,
        skin_data: SkinData,
        application_mode: enums.ApplicationMode = enums.ApplicationMode.order,
    ):
        handle = self.get_node_handle(node)

        if application_mode == enums.ApplicationMode.nearest:
            current_positions = self.get_vertex_positions(node)
            kd_tree = spatial.KDTree(skin_data.positions)
            _, new_indexes = kd_tree.query(current_positions)
            index_map = {
                old_index: new_index for old_index, new_index in enumerate(new_indexes)
            }
            index_map = typeguard.check_type(
                {
                    old_index: new_index
                    for old_index, new_index in enumerate(new_indexes)
                },
                dict[int, np.int64],
            )
            new_weights = np.array(
                tuple(skin_data.weights[index_map[index]] for index in index_map)
            )
            new_bone_ids = np.array(
                tuple(skin_data.bone_ids[index_map[index]] for index in index_map)
            )
            new_skin_data = skin_plus_plus.SkinData(
                skin_data.bone_names, new_bone_ids, new_weights, skin_data.positions
            )
            skin_data = new_skin_data
        print(handle)
        return self._apply_skin_data(handle, skin_data)
