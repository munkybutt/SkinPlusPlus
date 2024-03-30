from __future__ import annotations

import abc
import importlib
import pathlib


_typing = False
if _typing:

    from .. import _types
    from .. import SkinData
    from typing import Sequence

del _typing


class IHost(metaclass=abc.ABCMeta):
    _extract_skin_data: _types.T_EXSD
    _apply_skin_data: _types.T_CApSD
    _get_vertex_positions: _types.Callable

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
        # if is_reloading:
        #     importlib.reload(backend)

        self._extract_skin_data: _types.T_EXSD = backend.extract_skin_data
        self._apply_skin_data: _types.T_CApSD = backend.apply_skin_data
        self._get_vertex_positions: _types.Callable = backend.get_vertex_positions

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
    def get_selection(self) -> tuple[_types.T_Node, ...]:
        """
        Get the selection of the current host scene
        """

    @abc.abstractmethod
    def get_node_name(self, node: _types.T_Node) -> str:
        """
        Get the name of the given node
        """

    @abc.abstractmethod
    def get_node_handle(self, node: _types.T_Node) -> _types.T_Handle:
        """
        Get the unique handle of the given node
        """


    def extract_skin_data(self, node: _types.T_Node, vertex_ids: Sequence[int] | None = None) -> SkinData:
        handle = self.get_node_handle(node)
        return self._extract_skin_data(handle, vertex_ids=vertex_ids)

    def apply_skin_data(self, node: _types.T_Node, skin_data: SkinData):
        handle = self.get_node_handle(node)
        return self._apply_skin_data(handle, skin_data)