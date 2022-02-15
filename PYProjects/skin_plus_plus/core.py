from __future__ import annotations

# DO NOT REMOVE - Required for access to SkinData class:
from . import skin_plus_plus_py
from . import get_skin_data
# from . import get_vertex_positions
from . import set_skin_weights

import enum
import pickle
# import scipy.sparse
import pathlib

_typing = False
if _typing:
	pass
del _typing


class ImportType(enum.IntEnum):
	order = 0
	nearest = 1
	nearest_n = 2
	barycentric = 3


def export_skin_data(mesh_name: str, path: pathlib.Path):
	"""
	Get skin data from the given mesh and save it to disk.
	"""

	skin_data = get_skin_data(mesh_name)
	with open(path, "wb") as path_data:
		pickle.dump(skin_data, path_data)


def import_skin_data(mesh_name: str, path: pathlib.Path, import_type: ImportType = ImportType.order):
	"""
	Load skin data from disk and apply it to the given mesh.
	"""

	with open(path, "rb") as path_data:
		skin_data = pickle.load(path_data)

	return set_skin_weights(mesh_name, skin_data)

	# if import_type == ImportType.nearest:
	# 	vertex_positions = get_vertex_positions(mesh_name)
	# 	kd_tree = scipy.sparse.ckdtree(skin_data.positions)
	# 	matching_indices = kd_tree.query(vertex_positions)

