from __future__ import annotations

import json
import numpy
import pathlib
import pickle
import skin_plus_plus

from .enums import FileType
from .enums import ApplicationMode

_typing = False
if _typing:
    from . import _types
del _typing


def load_from_pickle_file(path: pathlib.Path) -> skin_plus_plus.SkinData:
    if path.suffix != ".skpp":
        path = path.with_suffix(".skpp")

    if not path.exists():
        raise IOError(f"File path does not exist: {path}")

    with open(path, "rb") as file:
        skin_data = pickle.load(file)

    assert isinstance(
        skin_data, skin_plus_plus.SkinData
    ), f"Incorrect data type loaded from pickle: {type(skin_data)}"
    return skin_data


def load_from_json_file(path: pathlib.Path):
    if path.suffix != ".skpp-json":
        path = path.with_suffix(".skpp-json")

    if not path.exists():
        raise IOError(f"File path does not exist: {path}")

    with open(path, "r") as file:
        data = json.load(file)
        return skin_plus_plus.SkinData(
            tuple(data["bone_names"]),
            tuple(data["bone_ids"]),
            tuple(data["weights"]),
            tuple(data["positions"]),
        )


def export_skin_data(
    node: _types.T_Node, path: pathlib.Path, file_type: FileType = FileType.pickle
):
    """
    Get skin data from the given mesh and save it to disk.
    """

    node_name = skin_plus_plus.current_host_interface.get_node_name(node)
    skin_data = skin_plus_plus.extract_skin_data(node)
    if not path.parent.exists():
        path.parent.mkdir(parents=True)

    if file_type == FileType.pickle:
        if path.suffix != ".skpp":
            path = path.with_suffix(".skpp")

        with open(path, "wb") as file:
            pickle.dump(skin_data, file)

    elif file_type == FileType.json:
        if path.suffix != ".skpp-json":
            path = path.with_suffix(".skpp-json")

        with open(path, "w") as file:
            weights = skin_data.weights.copy()
            weights[numpy.isnan(weights)] = 0
            _skin_data = {
                "bone_names": skin_data.bone_names,
                "bone_ids": skin_data.bone_ids.tolist(),
                "weights": weights.tolist(),
                "positions": skin_data.positions.tolist(),
                "vertex_ids": skin_data.vertex_ids.tolist()
                if hasattr(skin_data, "vertex_ids") and skin_data.vertex_ids
                else None,
            }
            json.dump(_skin_data, file, indent=4)

    print(f"Exported '{node_name}' skin data to: {path}")


def import_skin_data(
    node: _types.T_Node,
    path: pathlib.Path,
    file_type: FileType = FileType.pickle,
    import_type: ApplicationMode = ApplicationMode.order,
):
    """
    Load skin data from disk and apply it to the given mesh.
    """
    node_name = skin_plus_plus.current_host_interface.get_node_name(node)
    if file_type == FileType.pickle:
        try:
            skin_data = load_from_pickle_file(path)
        except IOError as error:
            if f"File path does not exist: {path}" in str(error):
                raise IOError(
                    f"File path does not exist: {path} - check mesh is named correctly: {node_name}"
                )

            raise

    elif file_type == FileType.json:
        try:
            skin_data = load_from_json_file(path)
        except IOError as error:
            if f"File path does not exist: {path}" in str(error):
                raise IOError(
                    f"File path does not exist: {path} - check mesh is named correctly: {node_name}"
                )

            raise

    return skin_plus_plus.apply_skin_data(node, skin_data, import_type=import_type)

    # if import_type == ImportType.nearest:
    #   vertex_positions = get_vertex_positions(node)
    #   kd_tree = scipy.sparse.ckdtree(skin_data.positions)
    #   matching_indices = kd_tree.query(vertex_positions)


if __name__ == "__main__":
    export_skin_data("test", pathlib.Path())
