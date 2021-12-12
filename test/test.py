from __future__ import annotations

import functools
import numpy as np
import pathlib
import pickle
import time

from pymxs import runtime as mxRt

_typing = False
if _typing:
    from typing import Any
    from typing import Callable
del _typing

_SKOPS = mxRt.SkinOps()
GetVertexWeight = _SKOPS.GetVertexWeight
GetVertexWeightCount = _SKOPS.GetVertexWeightCount
GetVertexWeightBoneID = _SKOPS.GetVertexWeightBoneID
ReplaceVertexWeights = _SKOPS.ReplaceVertexWeights


_SKINPPOPS = mxRt.SkinPPOps()
SKINPP_GetSkinWeights = mxRt.SkinPP.GetSkinWeights
SKINPPOPS_GetSkinWeights = _SKINPPOPS.GetSkinWeights
SPPGetSkinWeights = mxRt.SPPGetSkinWeights

SKINPP_SetSkinWeights = mxRt.SkinPP.SetSkinWeights
SKINPPOPS_SetSkinWeights = _SKINPPOPS.SetSkinWeights
SPPSetSkinWeights = mxRt.SPPSetSkinWeights


_MESHOP = mxRt.MeshOp()
MESHOP_GetVert = _MESHOP.GetVert

_POLYOP = mxRt.PolyOp()
POLYOP_GetVert = _POLYOP.GetVert

import site
# site.addsitedir(r"D:\Code\Git\SkinPlusPlus\PyModules\skin_plus_plus\x64\Debug")
site.addsitedir(r"D:\Code\Git\SkinPlusPlus\PyModules\skin_plus_plus\x64\Release")

import SkinPlusPlusPymxs

# skin_data = SkinPlusPlusPymxs.SkinData()
# skin_data.initialise("Sphere001")
# skin_weights = skin_data.getSkinWeights()
# print(skin_weights[0][1])


__loops__ = 1
def set_loops(value: int):
    global __loops__
    __loops__ = value


def get_loops():
    global __loops__
    return __loops__


def timer(data_dict: dict[str, tuple[float, Any, str]]) -> Callable:
    def wrapper(function: Callable) -> Callable:
        @functools.wraps(function)
        def wrapper_timer(*args, **kwargs) -> Any:
            start_time = time.perf_counter()
            value: Any = None
            for _ in range(get_loops()):
                value = function(*args, **kwargs)
            run_time = time.perf_counter() - start_time
            data_dict[f"{function.__name__!r}"] = (run_time, value, function.__name__)
            return value
        return wrapper_timer
    return wrapper


def getMxsFunctions():
    current_file = pathlib.Path(__file__)
    mxRt.FileIn(str(current_file.with_suffix(".ms")))
    return mxRt.mxsGetSkinWeights, mxRt.mxsSetSkinWeights


mxsGetSkinWeights, mxsSetSkinWeights = getMxsFunctions()


def GetSkinWeights(node) -> tuple[list[list[float]], list[list[int]], list[tuple[float, float, float]]]:
    skinModifier = node.Modifiers[mxRt.Skin]
    vertexCount = _SKOPS.GetNumberVertices(skinModifier)
    skinWeights: list[list[float]] = [None] * vertexCount
    skinBoneIDs: list[list[int]] = [None] * vertexCount
    positions: list[tuple[float, float, float]] = [None] * vertexCount

    getVertPosition = POLYOP_GetVert
    if mxRt.ClassOf(node) == mxRt.Editable_Mesh:
        getVertPosition = MESHOP_GetVert

    for vertexIndex in range(1, vertexCount + 1):
        influenceCount = GetVertexWeightCount(skinModifier, vertexIndex)
        vertexWeights:list[float] = [
            GetVertexWeight(skinModifier, vertexIndex, influenceIndex) for influenceIndex in range(1, influenceCount + 1)
        ]
        vertexBoneIDs: list[int] = [
            GetVertexWeightBoneID(skinModifier, vertexIndex, influenceIndex) for influenceIndex in range(1, influenceCount + 1)
        ]
        position = getVertPosition(node, vertexIndex)
        vertexIndexZero = vertexIndex - 1
        skinWeights[vertexIndexZero] = vertexWeights
        skinBoneIDs[vertexIndexZero] = vertexBoneIDs
        positions[vertexIndexZero] = (position.X, position.Y, position.Z)

    return skinWeights, skinBoneIDs, positions


def GetSkinWeights_NP(node) -> tuple[np.ndarray[float, Any], np.ndarray[int, Any], np.ndarray[float, Any]]:
    skinModifier = node.Modifiers[mxRt.Skin]
    vertexCount = _SKOPS.GetNumberVertices(skinModifier)
    getVertPosition = POLYOP_GetVert
    if mxRt.ClassOf(node) == mxRt.Editable_Mesh:
        getVertPosition = MESHOP_GetVert

    skinWeights: np.ndarray[float, Any] = np.empty((vertexCount, 6), dtype=float)
    skinBoneIDs: np.ndarray[float, Any] = np.empty((vertexCount, 6), dtype=int)
    positions: np.ndarray[float, Any] = np.empty((vertexCount, 3), dtype=float)
    for vertexIndex in range(1, vertexCount + 1):
        influenceCount = GetVertexWeightCount(skinModifier, vertexIndex)
        vertexWeights = np.array(
            [
                GetVertexWeight(skinModifier, vertexIndex, influenceIndex) for influenceIndex in range(1, influenceCount + 1)
            ],
            dtype=float
        )
        vertexBoneIDs = np.array(
            [
                GetVertexWeightBoneID(skinModifier, vertexIndex, influenceIndex) for influenceIndex in range(1, influenceCount + 1)
            ],
            dtype=float
        )
        mxs_position = list(getVertPosition(node, vertexIndex))
        position = np.array(mxs_position, dtype=float)
        vertexIndexZero = vertexIndex - 1
        skinWeights[vertexIndexZero] = vertexWeights
        skinBoneIDs[vertexIndexZero] = vertexBoneIDs
        positions[vertexIndexZero] = position

    return skinWeights, skinBoneIDs, positions


def SetSkinWeights(node, boneIDs, weights) -> None:
    skinModifier = node.Modifiers[mxRt.Skin]
    vertexCount = _SKOPS.GetNumberVertices(skinModifier) + 1
    for vertexIndex in range(1, vertexCount):
        vertexIndexZero = vertexIndex - 1
        ReplaceVertexWeights(skinModifier, vertexIndex, boneIDs[vertexIndexZero], weights[vertexIndexZero])


get_timer_dict: dict[str, tuple[float, Any, str]] = {}

@timer(get_timer_dict)
def mxs_GetSkinWeights(_obj):
    data = mxsGetSkinWeights(_obj)
    weights = [list(weights) for weights in data[0]]
    boneIDs = [list(boneIDs) for boneIDs in data[1]]
    positions = [list(positions) for positions in data[2]]

    return weights, boneIDs, positions


@timer(get_timer_dict)
def mxs_GetSkinWeights_NP(_obj):
    data = mxsGetSkinWeights(_obj)

    weights = np.array([np.array(list(weights), dtype=float) for weights in data[0]], dtype=float)
    boneIDs = np.array([np.array(list(boneIDs), dtype=float) for boneIDs in data[1]], dtype=float)
    positions = np.array([np.array(list(positions), dtype=float) for positions in data[2]], dtype=float)
    # boneIDs = [list(boneIDs) for boneIDs in data[1]]

    return weights, boneIDs, positions


@timer(get_timer_dict)
def pymxs_GetSkinWeights(_obj):
    return GetSkinWeights(_obj)


@timer(get_timer_dict)
def pymxs_GetSkinWeights_NP(_obj):
    return GetSkinWeights_NP(_obj)


@timer(get_timer_dict)
def cppfp_GetSkinWeights(_obj):
    data = SKINPP_GetSkinWeights(_obj)

    weights = np.array([list(weights) for weights in data[0]], dtype=float)
    boneIDs = np.array([list(boneIDs) for boneIDs in data[1]], dtype=int)
    positions = np.array([list(position) for position in data[2]], dtype=float)

    return weights, boneIDs, positions


@timer(get_timer_dict)
def cpppm_GetSkinWeights(_obj):
    data = SKINPPOPS_GetSkinWeights(_obj)

    weights = np.array([list(weights) for weights in data[0]], dtype=float)
    boneIDs = np.array([list(boneIDs) for boneIDs in data[1]], dtype=int)
    positions = np.array([list(position) for position in data[2]], dtype=float)

    return weights, boneIDs, positions


@timer(get_timer_dict)
def cpppf_GetSkinWeights(_obj):
    data = SPPGetSkinWeights(_obj)

    weights = np.array([list(weights) for weights in data[0]], dtype=float)
    boneIDs = np.array([list(boneIDs) for boneIDs in data[1]], dtype=int)
    positions = np.array([list(position) for position in data[2]], dtype=float)

    return weights, boneIDs, positions


@timer(get_timer_dict)
def pybind11_GetSkinWeights(_obj):
    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name, 0)


@timer(get_timer_dict)
def pybind11_GetSkinWeights_take_ownership(_obj):
    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name, 1)


@timer(get_timer_dict)
def pybind11_GetSkinWeights_copy(_obj):
    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name, 2)


@timer(get_timer_dict)
def pybind11_GetSkinWeights_move(_obj):
    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name, 3)


@timer(get_timer_dict)
def pybind11_GetSkinWeights_reference(_obj):
    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name, 4)


@timer(get_timer_dict)
def pybind11_GetSkinWeights_reference_internal(_obj):
    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name, 5)


@timer(get_timer_dict)
def pybind11_GetSkinWeights_automatic(_obj):
    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name, 6)


@timer(get_timer_dict)
def pybind11_GetSkinWeights_automatic_reference(_obj):
    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name, 7)


@timer(get_timer_dict)
def pybind11_GetData(_obj):
    return SkinPlusPlusPymxs.get_data(_obj.Name)

@timer(get_timer_dict)
def pybind11_GetData2(_obj):
    return SkinPlusPlusPymxs.get_data2()

set_timer_dict: dict[str, tuple[float, Any, str]] = {}

@timer(set_timer_dict)
def set_skin_weights(_obj, _boneIDs, _weights):
    SkinPlusPlusPymxs.set_skin_weights(
        _obj.Name,
        _boneIDs,
        _weights
    )


def as_mxs_array(value, dtype=float):
    mxsArray = mxRt.Array()
    array_length = len(value)
    mxsArray[array_length - 1] = None
    for index in range(array_length):
        sub_value = value[index]
        if isinstance(sub_value, np.ndarray):
            mxsArray[index] = as_mxs_array(sub_value, dtype=dtype)
        else:
            mxsArray[index] = dtype(sub_value)

    return mxsArray


@timer(set_timer_dict)
def mxs_SetSkinWeights(_obj, _boneIDs, _weights):
    mxsBoneIDs = as_mxs_array(_boneIDs, dtype=int)
    mxsWeights = as_mxs_array(_weights)
    return mxsSetSkinWeights(_obj, mxsBoneIDs, mxsWeights)


@timer(set_timer_dict)
def pymxs_SetSkinWeights(_obj, _boneIDs, _weights):
    mxsBoneIDs = as_mxs_array(_boneIDs, dtype=int)
    mxsWeights = as_mxs_array(_weights)
    return SetSkinWeights(_obj, mxsBoneIDs, mxsWeights)


@timer(set_timer_dict)
def cppfp_SetSkinWeights(_obj, _boneIDs, _weights):
    mxsBoneIDs = as_mxs_array(_boneIDs, dtype=int)
    mxsWeights = as_mxs_array(_weights)
    return SKINPP_SetSkinWeights(_obj, mxsBoneIDs, mxsWeights)


@timer(set_timer_dict)
def cpppm_SetSkinWeights(_obj, _boneIDs, _weights):
    mxsBoneIDs = as_mxs_array(_boneIDs, dtype=int)
    mxsWeights = as_mxs_array(_weights)
    return SKINPPOPS_SetSkinWeights(_obj, mxsBoneIDs, mxsWeights, [])


@timer(set_timer_dict)
def cpppf_SetSkinWeights(_obj, _boneIDs, _weights):
    mxsBoneIDs = as_mxs_array(_boneIDs, dtype=int)
    mxsWeights = as_mxs_array(_weights)
    return SPPSetSkinWeights(_obj, mxsBoneIDs, mxsWeights, [])


def run_functions(function_list, _obj, *args, loop_count: int = 1):

    set_loops(loop_count)
    for function in function_list:
        result = function(_obj, *args)
        if result is None:
            continue
        print(type(result))
        # print(len(result))


def process_results(time_data: dict[str, tuple[float, Any, str]]):
    # times = []
    # values = []
    data = list(time_data.values())
    data.sort(key=lambda x: x[0])
    max_time = data[-1][0]
    data.reverse()
    # max_time = max(times)
    for time, _, function_name in data:
        percentage_ratio = (max_time / time)
        message = f"{function_name}: {time} -"
        if percentage_ratio == 1.0:
            message = f"{message} base line"
        else:
            percentage_increase = (percentage_ratio * 100.0)
            message = f"{message} {percentage_ratio}x / {percentage_increase}% faster"
        print(message)


get_function_list = (
    pymxs_GetSkinWeights,
    pymxs_GetSkinWeights_NP,
    mxs_GetSkinWeights_NP,
    cppfp_GetSkinWeights,
    cpppm_GetSkinWeights,
    cpppf_GetSkinWeights,
    # pybind11_GetSkinWeights,
    # pybind11_GetSkinWeights_take_ownership,
    # pybind11_GetSkinWeights_copy,
    # pybind11_GetSkinWeights_move,
    # pybind11_GetSkinWeights_reference,
    # pybind11_GetSkinWeights_reference_internal,
    # pybind11_GetSkinWeights_automatic,
    # pybind11_GetSkinWeights_automatic_reference,
    # pybind11_GetData2,
    pybind11_GetData,
)

obj = mxRt.GetNodeByName("Sphere002")

# print(mxRt.Selection[0].Transform.Position)

run_functions(get_function_list, obj)
process_results(get_timer_dict)

# data = pybind11_GetData(obj)
# print(data)
# print(data.bone_ids)
# print(len(data.bone_ids))
# print(data.weights)
# print(len(data.weights))
# with open(r"D:\Code\Git\SkinPlusPlus\test\skin_data.pkl", "wb") as file:
#     pickle.dump(data, file)

# with open(r"D:\Code\Git\SkinPlusPlus\test\skin_data.pkl", "rb") as file:
#     pdata = pickle.load(file)

# print(pdata)
# print(pdata.bone_ids)
# print(len(pdata.bone_ids))
# print(pdata.weights)
# print(len(pdata.weights))


# print(pdata)
# print(data)
# print(data.bone_ids)
# print(data.weights)
# print(data.positions[20])

set_function_list = (
    mxs_SetSkinWeights,
    pymxs_SetSkinWeights,
    cppfp_SetSkinWeights,
    cpppm_SetSkinWeights,
    cpppf_SetSkinWeights,
    set_skin_weights
)


# vertex_count = obj.Verts.Count
# boneIDs = np.array([np.array([1.0, 2.0], dtype=np.float32) for _ in range(vertex_count)], dtype=np.float32)
# weights = np.array([np.array([0.5, 0.5], dtype=np.float32) for _ in range(vertex_count)], dtype=np.float32)

# run_functions(set_function_list, obj, boneIDs, weights)
# process_results(set_timer_dict)

# npa = np.array([[1.0, 2.0], [1.0, 2.0]], dtype=float)
# a1 = as_mxs_array(npa)
# a2 = as_mxs_array(npa, dtype=int)
# print(a1)
# print(a2)
# print(SkinPlusPlusPymxs.test_nested_np_array(npa))

# SkinPlusPlusPymxs