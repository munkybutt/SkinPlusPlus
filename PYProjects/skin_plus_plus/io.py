from __future__ import annotations

from . import core
from . import current_dcc
from . import current_host_interface
from .enums import FileType


def _get_clean_file_name(name: str, suffix: str | None = None) -> str:
    name = f"{name}{suffix}" if suffix and not name.endswith(suffix) else name
    return name.split(":")[-1]


def save(file_name_suffix: str = "_GEO", file_type: FileType = FileType.pickle):
    scene_path = current_host_interface.get_current_file_path()
    data_path = scene_path.parent / "_Data"
    selection = current_host_interface.get_selection()
    if not selection:
        raise RuntimeError("No nodes are selected!")

    for node in selection:
        node_name = current_host_interface.get_node_name(node)
        path_name = _get_clean_file_name(node_name, suffix=file_name_suffix)
        skin_data_path = data_path / f"{path_name}.skpp"
        core.export_skin_data(node_name, skin_data_path, file_type=file_type)


def load(file_name_suffix: str = "_GEO", file_type: FileType = FileType.pickle):
    scene_path = current_host_interface.get_current_file_path()
    data_path = scene_path.parent / "_Data"
    selection = current_host_interface.get_selection()
    if not selection:
        raise RuntimeError("No nodes are selected!")

    for node in selection:
        node_name = current_host_interface.get_node_name(node)
        path_name = _get_clean_file_name(node_name, suffix=file_name_suffix)
        skin_data_path = data_path / f"{path_name}.skpp"
        core.import_skin_data(node_name, skin_data_path, file_type=file_type)


def max_to_maya(file_type: FileType = FileType.pickle):
    if current_dcc == "max":
        return save(file_type=file_type)

    if current_dcc == "maya":
        return load(file_type=file_type)


def maya_to_max(file_type: FileType = FileType.pickle):
    if current_dcc == "maya":
        return save(file_type=file_type)

    if current_dcc == "max":
        return load(file_type=file_type)
