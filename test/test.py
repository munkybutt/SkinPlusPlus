from __future__ import annotations

import functools
import pathlib
import time

from pymxs import runtime as mxRt
from typing import Any
from typing import Callable

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

import site
site.addsitedir(r"D:\Code\Git\SkinPlusPlus\PyModules\skin_plus_plus\x64\Release")

import SkinPlusPlusPymxs

skin_data = SkinPlusPlusPymxs.SkinData()
skin_data.initialise("Sphere001")
skin_weights = skin_data.getSkinWeights()
print(skin_weights[0][1])


__loops__ = 1
def set_loops(value: int):
    global __loops__
    __loops__ = value

def get_loops():
    global __loops__
    return __loops__


def timer(data_dict: dict[str, tuple[float, Any]]) -> Callable:
    def wrapper(function: Callable) -> Callable:
        @functools.wraps(function)
        def wrapper_timer(*args, **kwargs) -> Any:
            start_time = time.perf_counter()
            value: Any = None
            for _ in range(get_loops()):
                value = function(*args, **kwargs)
            run_time = time.perf_counter() - start_time
            data_dict[f"{function.__name__!r}"] = (run_time, value)
            return value
        return wrapper_timer
    return wrapper

def getMxsFunctions():
    current_file = pathlib.Path(__file__)
    mxRt.FileIn(str(current_file.with_suffix(".ms")))
    return mxRt.mxsGetSkinWeights, mxRt.mxsSetSkinWeights

mxsGetSkinWeights, mxsSetSkinWeights = getMxsFunctions()

def GetSkinWeights(node) -> tuple[list[list[float]], list[list[int]]]:
    skinModifier = node.Modifiers[mxRt.Skin]
    vertexCount = _SKOPS.GetNumberVertices(skinModifier)
    skinWeights: list[list[float]] = []
    skinBoneIDs: list[list[int]] = []
    for vertexIndex in range(1, vertexCount + 1):
        influenceCount = GetVertexWeightCount(skinModifier, vertexIndex)
        vertexWeights:list[float] = [
            GetVertexWeight(skinModifier, vertexIndex, influenceIndex) for influenceIndex in range(1, influenceCount + 1)
        ]
        vertexBoneIDs: list[int] = [
            GetVertexWeightBoneID(skinModifier, vertexIndex, influenceIndex) for influenceIndex in range(1, influenceCount + 1)
        ]
        skinWeights.append(vertexWeights)
        skinBoneIDs.append(vertexBoneIDs)

    return skinWeights, skinBoneIDs

def SetSkinWeights(node, boneIDs, weights) -> None:
    skinModifier = node.Modifiers[mxRt.Skin]
    vertexCount = _SKOPS.GetNumberVertices(skinModifier)
    for vertexIndex in range(vertexCount, 1):
        ReplaceVertexWeights(skinModifier, vertexIndex, boneIDs[vertexIndex], weights[vertexIndex])


get_timer_dict: dict[str, tuple[float, Any]] = {}
@timer(get_timer_dict)
def mxs_GetSkinWeights(_obj):
    data = mxsGetSkinWeights(_obj)
    weights = [list(weights) for weights in data[0]]
    boneIDs = [list(boneIDs) for boneIDs in data[1]]

    return weights, boneIDs


@timer(get_timer_dict)
def pymxs_GetSkinWeights(_obj):
    return GetSkinWeights(_obj)

@timer(get_timer_dict)
def cppfp_GetSkinWeights(_obj):
    data = SKINPP_GetSkinWeights(_obj)
    weights = [list(weights) for weights in data[0]]
    boneIDs = [list(boneIDs) for boneIDs in data[1]]

    return weights, boneIDs

@timer(get_timer_dict)
def cpppm_GetSkinWeights(_obj):
    data = SKINPPOPS_GetSkinWeights(_obj)
    weights = [list(weights) for weights in data[0]]
    boneIDs = [list(boneIDs) for boneIDs in data[1]]

    return weights, boneIDs

@timer(get_timer_dict)
def cpppf_GetSkinWeights(_obj):
    data = SPPGetSkinWeights(_obj)

    weights = [list(weights) for weights in data[0]]
    boneIDs = [list(boneIDs) for boneIDs in data[1]]

    return weights, boneIDs

@timer(get_timer_dict)
def pybind11_GetSkinWeights(_obj):

    return SkinPlusPlusPymxs.get_skin_weights(_obj.Name)

@timer(get_timer_dict)
def pybind11np_GetSkinWeights(_obj):

    return SkinPlusPlusPymxs.get_skin_weights_np(_obj.Name)

@timer(get_timer_dict)
def pybind11npmove_GetSkinWeights(_obj):

    return SkinPlusPlusPymxs.get_skin_weights_np_move(_obj.Name)

@timer(get_timer_dict)
def pybind11nptakeownership_GetSkinWeights(_obj):

    return SkinPlusPlusPymxs.get_skin_weights_np_take_ownership(_obj.Name)


set_timer_dict: dict[str, tuple[float, Any]] = {}
@timer(set_timer_dict)
def mxs_SetSkinWeights( _obj, _boneIDs, _weights):
    return SetSkinWeights(_obj, _boneIDs, _weights)

@timer(set_timer_dict)
def cppfp_SetSkinWeights( _obj, _boneIDs, _weights):
    return SKINPP_SetSkinWeights(_obj, _boneIDs, _weights)

@timer(set_timer_dict)
def cpppm_SetSkinWeights( _obj, _boneIDs, _weights):
    return SKINPPOPS_SetSkinWeights(_obj, _boneIDs, _weights, [])

@timer(set_timer_dict)
def cpppf_SetSkinWeights( _obj, _boneIDs, _weights):
    return SPPSetSkinWeights(_obj, _boneIDs, _weights, [])


obj = mxRt.GetNodeByName("Sphere001")

get_function_list = (
    # pymxs_GetSkinWeights,
    # mxs_GetSkinWeights,
    # cppfp_GetSkinWeights,
    # cpppm_GetSkinWeights,
    # cpppf_GetSkinWeights,
    pybind11_GetSkinWeights,
    pybind11np_GetSkinWeights,
    pybind11npmove_GetSkinWeights,
    pybind11nptakeownership_GetSkinWeights,
    # lambda: pybind11np_GetSkinWeights(obj),
)

# weights = [(0.5, 0.5) for _ in range(obj.Verts.Count)]
# ids = [(1, 2) for _ in range(obj.Verts.Count)]
# set_function_list = (
#     lambda: mxs_SetSkinWeights(obj, weights, ids),
#     lambda: cppfp_SetSkinWeights(obj, weights, ids),
#     lambda: cpppm_SetSkinWeights(obj, weights, ids),
#     lambda: cpppf_SetSkinWeights(obj, weights, ids),
# )

set_loops(1)
def run_functions(function_list, _obj):
    for function in function_list:
        result = function(_obj)
        if result is None:
            continue
        print(type(result))
        print(len(result))

def process_results(time_data: dict[str, tuple[float, Any]]):
    times = []
    values = []
    for time, value  in time_data.values():
        times.append(time)
        values.append(value)

    max_time = max(times)
    for function_name, (time, value) in time_data.items():
        percentage_ratio = (max_time / time)
        message = f"{function_name}: {time} -"
        if percentage_ratio == 1.0:
            message = f"{message} base line"
        else:
            percentage_increase = (percentage_ratio * 100.0)
            message = f"{message} {percentage_ratio}x / {percentage_increase}% faster"
        print(message)

run_functions(get_function_list, obj)
process_results(get_timer_dict)

# print(np_GetSkinWeights())
