from __future__ import annotations

import numpy as np
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


get_timer_dict: dict[str, tuple[float, Any, str]] = {}
@timer(get_timer_dict)
def mxs_GetSkinWeights(_obj):
    data = mxsGetSkinWeights(_obj)
    weights = [list(weights) for weights in data[0]]
    boneIDs = [list(boneIDs) for boneIDs in data[1]]

    return weights, boneIDs

@timer(get_timer_dict)
def mxs_GetSkinWeights_NP(_obj):
    data = mxsGetSkinWeights(_obj)

    weights = np.array([np.array(list(weights), dtype=float) for weights in data[0]], dtype=float)
    boneIDs = np.array([np.array(list(boneIDs), dtype=float) for boneIDs in data[1]], dtype=float)
    # boneIDs = [list(boneIDs) for boneIDs in data[1]]

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

set_timer_dict: dict[str, tuple[float, Any, str]] = {}
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
    pymxs_GetSkinWeights,
    mxs_GetSkinWeights,
    mxs_GetSkinWeights_NP,
    cppfp_GetSkinWeights,
    cpppm_GetSkinWeights,
    cpppf_GetSkinWeights,
    pybind11_GetSkinWeights,
    pybind11_GetSkinWeights_take_ownership,
    pybind11_GetSkinWeights_copy,
    pybind11_GetSkinWeights_move,
    pybind11_GetSkinWeights_reference,
    pybind11_GetSkinWeights_reference_internal,
    pybind11_GetSkinWeights_automatic,
    pybind11_GetSkinWeights_automatic_reference,
)


set_loops(1)
def run_functions(function_list, _obj):
    for function in function_list:
        result = function(_obj)
        if result is None:
            continue
        print(type(result))
        print(len(result))

def process_results(time_data: "dict[str, tuple[float, Any, str]]"):
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

run_functions(get_function_list, obj)
process_results(get_timer_dict)


| Function                                    | Time-secs           | x Faster             | % Faster             |
|---------------------------------------------|---------------------|----------------------|----------------------|
| pymxs_GetSkinWeights                        | 20.34769090000009   | base line            | base line            |
| mxs_GetSkinWeights_NP                       | 15.51825759999997   | 1.3112097649416599 x | 131.12097649416597 % |
| mxs_GetSkinWeights                          | 14.42323169999986   | 1.4107580966060669 x | 141.0758096606067 %  |
| cpppf_GetSkinWeights                        | 7.435437399999955   | 2.7365829076847867 x | 273.65829076847865 % |
| cppfp_GetSkinWeights                        | 6.338866400000143   | 3.2099889185232917 x | 320.99889185232917 % |
| cpppm_GetSkinWeights                        | 5.98266609999996    | 3.4011075597216136 x | 340.11075597216137 % |
| pybind11_GetSkinWeights_automatic           | 1.2681291999999758  | 16.045439928360988 x | 1604.5439928360988 % |
| pybind11_GetSkinWeights_move                | 1.09791139999993    | 18.533090101807293 x | 1853.3090101807293 % |
| pybind11_GetSkinWeights_copy                | 0.9864563000000999  | 20.627057579740764 x | 2062.7057579740763 % |
| pybind11_GetSkinWeights                     | 0.9028401000000486  | 22.537424844110262 x | 2253.7424844110265 % |
| pybind11_GetSkinWeights_reference_internal  | 0.4243109000001368  | 47.954674037347445 x | 4795.467403734745 %  |
| pybind11_GetSkinWeights_automatic_reference | 0.4236172999999326  | 48.03319151508526 x  | 4803.3191515085255 % |
| pybind11_GetSkinWeights_take_ownership      | 0.41753419999986363 | 48.73299217167536 x  | 4873.299217167536 %  |
| pybind11_GetSkinWeights_reference           | 0.41740709999999126 | 48.747831313843285 x | 4874.783131384329 %  |
