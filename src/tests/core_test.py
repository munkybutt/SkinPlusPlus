from __future__ import annotations

import importlib
import random

import numpy as np
from pymxs import runtime as mxrt


def __setup__():
    import inspect
    import pathlib
    import site

    current_directory = pathlib.Path(inspect.getfile(lambda: None)).parent
    spp_directory = current_directory.parent
    venv_directory = spp_directory.parent / ".dev/venv/310/.venv/Lib/site-packages"
    site.addsitedir(spp_directory.as_posix())
    site.addsitedir(venv_directory.as_posix())

__setup__()

import skin_plus_plus  # noqa: E402


def test_random_skin_data():
    length = random.randint(1, 2)
    width = random.randint(1, 2)
    bone_count = random.randint(2, 3)
    plane = mxrt.Plane(LengthSegs=length, WidthSegs=width)
    mxrt.ConvertTo(plane, mxrt.Editable_Poly)
    vert_count = mxrt.PolyOp.GetNumVerts(plane)
    bones: list[mxrt.Point] = []
    bone_names: list[str] = []
    for index in range(bone_count):
        x = random.uniform(-10.0, 10.0)
        y = random.uniform(-10.0, 10.0)
        z = random.uniform(-10.0, 10.0)
        bone_name = f"Bone_{index}"
        bone_names.append(bone_name)
        bone = mxrt.Point(Name=bone_name, Position=mxrt.Point3(x, y, z))
        bones.append(bone)

    weights = []
    bone_ids = []
    positions = []
    for _ in range(vert_count):
        weight_count = random.randint(1, bone_count)
        vertex_weights = [random.random() for _ in range(weight_count)]
        difference = bone_count - weight_count
        total_weight = sum(vertex_weights)
        vertex_bone_ids = np.random.choice(
            np.arange(0, bone_count), size=weight_count, replace=False
        )
        positions.append(np.zeros(3))
        for _ in range(difference):
            vertex_weights.append(0.0)
            vertex_bone_ids = np.append(vertex_bone_ids, -1)
        vertex_weights = np.array([float(weight) / total_weight for weight in vertex_weights])
        weights.append(vertex_weights)
        bone_ids.append(vertex_bone_ids)

    skin = mxrt.Skin()
    mxrt.AddModifier(plane, skin)

    bone_ids_np = np.array(bone_ids)
    weights_np = np.array(weights)
    positions_np = np.array(positions)

    print(bone_ids_np)
    print(weights_np)
    print(positions_np)
    skin_data = skin_plus_plus.SkinData(
        bone_names,
        bone_ids_np,
        weights_np,
        positions_np
    )
    print(skin_data)

    skin_plus_plus.apply_skin_data(plane, skin_data)

    # print(weights)
    # print(bone_ids)


if __name__ == "__main__":
    print(skin_plus_plus)
    # test_random_skin_data()
    skin_plus_plus.extract_skin_data(mxrt.Selection[0])
