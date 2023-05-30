from __future__ import annotations

from . import get_skin_data

# from . import get_vertex_positions
from . import set_skin_weights

import enum
import json
import numpy
import pathlib
import pickle
import skin_plus_plus

# import scipy.sparse

_typing = False
if _typing:
    pass
del _typing


class ImportType(enum.Enum):
    order = 0
    nearest = 1
    nearest_n = 2
    barycentric = 3


class FileType(enum.Enum):
    """
    Enum to specify the type of file to save skin data as.

    Arguments:
    ----------

    - `json`
    - `pickle`
    """
    json = 0
    pickle = 1


def export_skin_data(mesh_name: str, path: pathlib.Path, file_type: FileType = FileType.pickle):
    """
    Get skin data from the given mesh and save it to disk.
    """

    skin_data = get_skin_data(mesh_name)
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
            }
            json.dump(_skin_data, file, indent=4)

    print(f"Exported '{mesh_name}' skin data to: {path}")


def import_skin_data(
    mesh_name: str,
    path: pathlib.Path,
    file_type: FileType = FileType.pickle,
    import_type: ImportType = ImportType.order,
):
    """
    Load skin data from disk and apply it to the given mesh.
    """

    if file_type == FileType.pickle:
        if path.suffix != ".skpp":
            path = path.with_suffix(".skpp")

        if not path.exists():
            raise IOError(
                f"File path does not exist: {path} - check mesh is named correctly: {mesh_name}"
            )

        with open(path, "rb") as file:
            skin_data = pickle.load(file)

    elif file_type == FileType.json:
        if path.suffix != ".skpp-json":
            path = path.with_suffix(".skpp-json")

        if not path.exists():
            raise IOError(
                f"File path does not exist: {path} - check mesh is named correctly: {mesh_name}"
            )

        with open(path, "r") as file:
            data = json.load(file)
            skin_data = skin_plus_plus.skin_plus_plus_py.SkinData(
                tuple(data["bone_names"]),
                tuple(data["bone_ids"]),
                tuple(data["weights"]),
                tuple(data["positions"])
            )

    return set_skin_weights(mesh_name, skin_data)

    # if import_type == ImportType.nearest:
    #   vertex_positions = get_vertex_positions(mesh_name)
    #   kd_tree = scipy.sparse.ckdtree(skin_data.positions)
    #   matching_indices = kd_tree.query(vertex_positions)
