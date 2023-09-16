from __future__ import annotations


import inspect
import pathlib
import site

if __name__ == "__main__":

    def __setup__():
        current_file = pathlib.Path(inspect.getfile(lambda: None))
        if str(current_file) == "<maya console>":
            # maya is a piece of shit:
            current_file = pathlib.Path(
                r"D:\Code\Git\SkinPlusPlus\PYProjects\skin_plus_plus_test\skin_plus_plus_test.py"
            )
        current_directory = current_file.parent
        site.addsitedir(str(current_directory.parent))

    __setup__()

import functools
import numpy as np
import random
import sys
import time
import unittest


import skin_plus_plus

# skin_plus_plus.set_debug(False)

if __name__ == "__main__":
    from importlib import reload
    # reload(skin_plus_plus.core)
    # reload(skin_plus_plus.io)
    # reload(skin_plus_plus)

_typing = False
if _typing:
    from typing import Any
    from typing import Callable
del _typing


_loops = 1


def set_loops(value: int):
    global _loops
    _loops = value


def get_loops() -> int:
    global _loops
    return _loops


def timer(data_dict: dict[str, tuple[float, Any, str]]) -> Callable:
    def wrapper(function: Callable) -> Callable:
        @functools.wraps(function)
        def wrapper_timer(*args, **kwargs) -> Any:
            start_time = time.perf_counter()
            value: Any = None
            for _ in range(get_loops()):
                value = function(*args, **kwargs)
            run_time = time.perf_counter() - start_time
            data_dict[f"{repr(function.__name__)}"] = (run_time, value, function.__name__)
            return value
        return wrapper_timer
    return wrapper


class SkinPlusPlusTestBase(unittest.TestCase):

    nodes_to_delete = []

    @classmethod
    def setUpClass(cls):
        try:
            file = __file__
        except NameError:
            file = r"D:\Code\Git\SkinPlusPlus\PYProjects\skin_plus_plus_test\skin_plus_plus_test.py"
        cls._current_directory = pathlib.Path(file).parent
        cls._dcc_test_files_directory = cls._current_directory / "dcc_test_files"
        cls._skin_data_file = cls._dcc_test_files_directory / "test_skin_data.sknd"

    @classmethod
    def tearDownClass(cls):
        # print("tearDownClass")
        # print(f"cls.nodes_to_delete: {cls.nodes_to_delete}")
        cls.delete_nodes(cls.nodes_to_delete)

    @classmethod
    def delete_nodes(cls, nodes):
        raise NotImplementedError()

    @staticmethod
    def run_functions(function_list, _obj, *args, loop_count: int = 1):
        set_loops(loop_count)
        for function in function_list:
            result = function(_obj, *args)
            if result is None:
                continue
            print(type(result))
            # print(len(result))

    @staticmethod
    def process_results(time_data: dict[str, tuple[float, Any, str]]):
        data = list(time_data.values())
        data.sort(key=lambda x: x[0])
        max_time = data[-1][0]
        data.reverse()
        for time, _, function_name in data:
            percentage_ratio = (max_time / time)
            message = f"{function_name}: {time} -"
            if percentage_ratio == 1.0:
                message = f"{message} base line"
            else:
                percentage_increase = (percentage_ratio * 100.0)
                message = f"{message} {percentage_ratio}x / {percentage_increase}% faster"
            print(message)

    def test_export_skin_data(self):
        skin_plus_plus.export_skin_data("test_mesh", self._skin_data_file)

    def test_import_skin_data(self):
        skin_plus_plus.import_skin_data("test_mesh", self._skin_data_file)


class SkinPlusPlusTestMax(SkinPlusPlusTestBase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        executable = sys.executable.lower()
        assert "3ds max" in executable
        cls.setup_mxs_environment()

    @classmethod
    def delete_nodes(cls, nodes):
        return cls.mxRt.Delete(nodes)

    @classmethod
    def setup_mxs_environment(cls):
        from pymxs import runtime as mxRt

        cls.mxRt = mxRt

        skinOps = mxRt.SkinOps()
        cls.skinOps_GetNumberVertices = skinOps.GetNumberVertices
        cls.skinOps_GetVertexWeight = skinOps.GetVertexWeight
        cls.skinOps_GetVertexWeightCount = skinOps.GetVertexWeightCount
        cls.skinOps_GetVertexWeightBoneID = skinOps.GetVertexWeightBoneID
        cls.skinOps_ReplaceVertexWeights = skinOps.ReplaceVertexWeights

        cls.skinPPOps = mxRt.SkinPPOps()
        cls.sdk_primative_method_get_skin_weights = cls.skinPPOps.GetSkinWeights
        cls.SkinPP_GetSkinWeights = mxRt.SkinPP.GetSkinWeights
        cls.SPP_GetSkinWeights = mxRt.SPPGetSkinWeights

        cls.SKINPP_SetSkinWeights = mxRt.SkinPP.SetSkinWeights
        cls.SKINPPOPS_SetSkinWeights = cls.skinPPOps.SetSkinWeights
        cls.SPPSetSkinWeights = mxRt.SPPSetSkinWeights

        meshOp = mxRt.MeshOp()
        cls.meshOp_GetVert = meshOp.GetVert

        polyOp = mxRt.PolyOp()
        cls.polyOp_GetVert = polyOp.GetVert

        cls.mxs_get_skin_weights, cls.mxs_set_skin_weights = cls.get_mxs_functions()

        cls.skinned_sphere = mxRt.Sphere(Name="SkinnedSphere")
        cls.skin_modifier = mxRt.Skin()
        mxRt.AddModifier(cls.skinned_sphere, cls.skin_modifier)
        cls.skinned_points = []
        for num in range(4):
            skinned_point = mxRt.Point(Name=f"SkinnedPoint_{num}")
            update = -1 if num < 3 else 0
            skinOps.AddBone(cls.skin_modifier, skinned_point, update)
            cls.skinned_points.append(skinned_point)

        cls.nodes_to_delete.append(cls.skinned_sphere)
        cls.nodes_to_delete.extend(cls.skinned_points)
        cls.weights = np.zeros((cls.skinned_sphere.Verts.Count, 4), dtype=float)
        for index in range(cls.skinned_sphere.Verts.Count):
            weights = [random.random() for _ in range(4)]
            weights_total = sum(weights)
            weights = [float(i) / weights_total for i in weights]
            cls.weights[index] = np.array(weights, dtype=float)
            cls.skinOps_ReplaceVertexWeights(cls.skin_modifier, index + 1, [1, 2, 3, 4], weights)

    @classmethod
    def get_mxs_functions(cls):
        current_file = pathlib.Path(__file__)
        cls.mxRt.FileIn(str(current_file.with_suffix(".ms")))
        return cls.mxRt.mxsGetSkinWeights, cls.mxRt.mxsSetSkinWeights

    def test_get_performance(self):

        return
        get_timer_dict: dict[str, tuple[float, Any, str]] = {}

        @timer(get_timer_dict)
        def mxs_get_skin_weights(_obj):
            data = self.mxs_get_skin_weights(_obj)
            weights = np.array(
                [np.array(list(weights), dtype=float) for weights in data[0]], dtype=float
            )
            boneIDs = np.array(
                [np.array(list(boneIDs), dtype=float) for boneIDs in data[1]], dtype=float
            )
            positions = np.array(
                [np.array(list(positions), dtype=float) for positions in data[2]], dtype=float
            )

            return weights, boneIDs, positions


        @timer(get_timer_dict)
        def pymxs_get_skin_weights(_obj) -> tuple[np.ndarray[float, Any], np.ndarray[int, Any], np.ndarray[float, Any]]:
            skinModifier = _obj.Modifiers[self.mxRt.Skin]
            vertexCount = self.skinOps_GetNumberVertices(skinModifier)
            getVertPosition = self.polyOp_GetVert
            if self.mxRt.ClassOf(_obj) == self.mxRt.Editable_Mesh:
                getVertPosition = self.meshOp_GetVert

            skinWeights: np.ndarray[float, Any] = np.empty((vertexCount, 6), dtype=float)
            skinBoneIDs: np.ndarray[float, Any] = np.empty((vertexCount, 6), dtype=int)
            positions: np.ndarray[float, Any] = np.empty((vertexCount, 3), dtype=float)
            for vertexIndex in range(1, vertexCount + 1):
                influenceCount = self.skinOps_GetVertexWeightCount(skinModifier, vertexIndex)
                vertexWeights = np.array(
                    [
                        self.skinOps_GetVertexWeight(skinModifier, vertexIndex, influenceIndex)
                        for influenceIndex in range(1, influenceCount + 1)
                    ],
                    dtype=float
                )
                vertexBoneIDs = np.array(
                    [
                        self.skinOps_GetVertexWeightBoneID(skinModifier, vertexIndex, influenceIndex)
                        for influenceIndex in range(1, influenceCount + 1)
                    ],
                    dtype=float
                )
                mxs_position = list(getVertPosition(_obj, vertexIndex))
                position = np.array(mxs_position, dtype=float)
                vertexIndexZero = vertexIndex - 1
                skinWeights[vertexIndexZero] = vertexWeights
                skinBoneIDs[vertexIndexZero] = vertexBoneIDs
                positions[vertexIndexZero] = position

            return skinWeights, skinBoneIDs, positions


        @timer(get_timer_dict)
        def sdk_primative_method_get_skin_weights(_obj):
            weights, boneIDs, positions = self.sdk_primative_method_get_skin_weights(_obj)

            weights = np.array([list(weights) for weights in weights], dtype=float)
            boneIDs = np.array([list(boneIDs) for boneIDs in boneIDs], dtype=int)
            positions = np.array([list(position) for position in positions], dtype=float)

            return weights, boneIDs, positions


        @timer(get_timer_dict)
        def skin_plus_plus_get_skin_data(_obj):
            return skin_plus_plus.get_skin_data(_obj.Name)


        get_function_list = (
            mxs_get_skin_weights,
            pymxs_get_skin_weights,
            sdk_primative_method_get_skin_weights,
            skin_plus_plus_get_skin_data,
        )

        obj = self.mxRt.GetNodeByName("test_mesh_high")
        self.run_functions(get_function_list, obj)
        self.process_results(get_timer_dict)

    def test_get_skin_data(self):
        skin_data = skin_plus_plus.get_skin_data(self.skinned_sphere.Name)
        # because the values returned from 3ds max are 32bit, there can be rounding errors
        # which can cause a direct comparison to fail, so we use np.allclose instead:
        # https://numpy.org/doc/stable/reference/generated/numpy.allclose.html#numpy.allclose
        self.assertTrue(np.allclose(skin_data.weights, self.weights))

    def test_set_performance(self):
        return
        def _as_mxs_array(value, dtype=float):
            mxsArray = self.mxRt.Array()
            array_length = len(value)
            mxsArray[array_length - 1] = None
            for index in range(array_length):
                sub_value = value[index]
                if isinstance(sub_value, np.ndarray):
                    mxsArray[index] = _as_mxs_array(sub_value, dtype=dtype)
                else:
                    mxsArray[index] = dtype(sub_value)

            return mxsArray

        set_timer_dict: dict[str, tuple[float, Any, str]] = {}

        def SetSkinWeights(_obj, _bone_ids, _weights) -> None:
            skinModifier = _obj.Modifiers[self.mxRt.Skin]
            vertexCount = self.skinOps_GetNumberVertices(skinModifier) + 1
            for vertex_index in range(1, vertexCount):
                vertex_index_zero = vertex_index - 1
                self.skinOps_ReplaceVertexWeights(
                    skinModifier, vertex_index, _bone_ids[vertex_index_zero], _weights[vertex_index_zero]
                )

        @timer(set_timer_dict)
        def set_skin_weights(_obj, _bone_ids, _weights):
            skin_plus_plus.set_skin_weights(
                _obj.Name,
                _bone_ids,
                _weights
            )

        @timer(set_timer_dict)
        def mxs_SetSkinWeights(_obj, _bone_ids, _weights):
            mxsBoneIDs = _as_mxs_array(_bone_ids, dtype=int)
            mxsWeights = _as_mxs_array(_weights)
            return self.mxs_set_skin_weights(_obj, mxsBoneIDs, mxsWeights)

        @timer(set_timer_dict)
        def pymxs_SetSkinWeights(_obj, _bone_ids, _weights):
            mxsBoneIDs = _as_mxs_array(_bone_ids, dtype=int)
            mxsWeights = _as_mxs_array(_weights)
            return SetSkinWeights(_obj, mxsBoneIDs, mxsWeights)

        @timer(set_timer_dict)
        def cppfp_SetSkinWeights(_obj, _bone_ids, _weights):
            mxsBoneIDs = _as_mxs_array(_bone_ids, dtype=int)
            mxsWeights = _as_mxs_array(_weights)
            return self.SKINPP_SetSkinWeights(_obj, mxsBoneIDs, mxsWeights)

        @timer(set_timer_dict)
        def cpppm_SetSkinWeights(_obj, _bone_ids, _weights):
            mxsBoneIDs = _as_mxs_array(_bone_ids, dtype=int)
            mxsWeights = _as_mxs_array(_weights)
            return self.SKINPPOPS_SetSkinWeights(_obj, mxsBoneIDs, mxsWeights, [])

        @timer(set_timer_dict)
        def cpppf_SetSkinWeights(_obj, _bone_ids, _weights):
            mxsBoneIDs = _as_mxs_array(_bone_ids, dtype=int)
            mxsWeights = _as_mxs_array(_weights)
            return self.SPPSetSkinWeights(_obj, mxsBoneIDs, mxsWeights, [])

        set_function_list = (
            SetSkinWeights,
            set_skin_weights,
            mxs_SetSkinWeights,
            pymxs_SetSkinWeights,
            cppfp_SetSkinWeights,
            cpppm_SetSkinWeights,
            cpppf_SetSkinWeights
        )

        obj = self.mxRt.GetNodeByName("test_mesh_high")
        self.run_functions(set_function_list, obj)
        self.process_results(set_timer_dict)


class SkinPlusPlusTestMaya(SkinPlusPlusTestBase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        executable = sys.executable.lower()
        assert "maya" in executable

        from maya import cmds
        from maya import mel

        import maya.api.OpenMaya as om
        import maya.api.OpenMayaAnim as oman
        import pymel.core as pm

        cls.cmds = cmds
        cls.mel = mel
        cls.om = om
        cls.oman = oman
        cls.pm = pm

        # version_info = cls.cmds.MaxVersion()
        # max_file_path = pathlib.Path(
        #     f"{cls._dcc_test_files_directory}/max/test_skin_data_{version_info[0]}.max"
        # )
        # current_max_file_path = pathlib.Path(cls.cmds.MaxFilePath) / cls.cmds.MaxFileName
        # if current_max_file_path == max_file_path:
        #     return

        # if not max_file_path.exists():
        #     raise FileNotFoundError(f"No test file for current max version:\n - {max_file_path}")

        # cls.cmds.LoadMaxFile(str(max_file_path))

    @classmethod
    def setup_maya_environment(cls):
        pass

    def test_get_performance(self):
        get_timer_dict: dict[str, tuple[float, Any, str]] = {}

        def find_related_skin_cluster(geometry):
            if not self.cmds.objExists(geometry):
                raise Exception(f"Object {geometry} does not exist!")

            if self.cmds.objectType(geometry) == "transform":
                try: geometry = self.cmds.listRelatives(geometry, s=True, ni=True, pa=True)[0]
                except: raise Exception(f"Object {geometry} has no deformable geometry!")

            skin = self.mel.eval(f"findRelatedSkinCluster '{geometry}'")
            if not skin:
                skin = self.cmds.ls(self.cmds.listHistory(geometry),type='skinCluster')
                skin = skin[0] if skin else None

            if not skin:
                raise RuntimeError(f"{geometry} has no skin cluster!")

            return skin

        @timer(get_timer_dict)
        def cmds_get_skin_weights(_obj: str):
            vertex_count = self.cmds.polyEvaluate(_obj, vertex=True)
            skin_cluster = find_related_skin_cluster(_obj)
            skin_weights = np.empty((vertex_count, 6), dtype=float)
            skin_bone_names = [None] * vertex_count
            positions: np.ndarray[float, Any] = np.empty((vertex_count, 3), dtype=float)
            for vertex_index in range(vertex_count):
                vertex_index_str = f"{_obj}.vtx[{vertex_index}]"
                vertex_weights = np.array(
                    self.cmds.skinPercent(
                        skin_cluster,
                        vertex_index_str,
                        query=True,
                        value=True,
                        ib=0.00001
                    ),
                    dtype=float
                )
                bone_names = self.cmds.skinPercent(
                    skin_cluster,
                    vertex_index_str,
                    transform=None,
                    query=True,
                    ib=0.00001
                )
                positions[vertex_index] = np.array(
                    self.cmds.xform(
                        vertex_index_str,
                        query=True,
                        translation=True,
                        worldSpace=False
                    ),
                    dtype=float
                )
                skin_weights[vertex_index] = vertex_weights
                skin_bone_names[vertex_index] = bone_names

            return skin_weights, positions, skin_bone_names

        @timer(get_timer_dict)
        def pymel_get_skin_weights(_obj: str):

            def get_skin_cluster(__obj):
                """Get the skincluster of a given object
                Arguments:
                    __obj (dagNode): The object to get skincluster
                Returns:
                    pyNode: The skin cluster pynode object
                """

                skin_cluster = None
                if isinstance(__obj, str):
                    __obj = self.pm.PyNode(__obj)
                try:
                    if self.pm.nodeType(__obj.getShape()) not in ("mesh", "nurbsSurface", "nurbsCurve"):
                        return
                    for shape in __obj.getShapes():
                        try:
                            for _skin_cluster in self.pm.listHistory(shape, type="skinCluster"):
                                try:
                                    if _skin_cluster.getGeometry()[0] == shape:
                                        skin_cluster = _skin_cluster
                                except IndexError:
                                    pass
                        except Exception:
                            pass
                except Exception:
                    self.pm.displayWarning("%s: is not supported." % __obj.name())

                return skin_cluster

            def get_mesh_components_from_tag_expression(skin_cls, tag="*"):
                """Get the mesh components from the  component tag expression
                Thanks to Roy Nieterau a.k.a BigRoyNL from colorBleed for the snippet
                Args:
                    skin_cls (PyNode): Skin cluster node
                    tag (str, optional): Component tag expression
                Returns:
                    dag_path, MObject: The dagpath tho the shpe and the MObject components
                """
                mesh_types = ("mesh", "nurbsSurface", "nurbsCurve")
                mesh = None
                for mesh_type in mesh_types:
                    obj = skin_cls.listConnections(et=True, type=mesh_type)
                    if not obj:
                        continue
                    mesh = obj[0].getShape().name()

                if mesh is None:
                    raise RuntimeError("No mesh found!")

                # Get the mesh out attribute for the shape
                out_attr = self.cmds.deformableShape(mesh, localShapeOutAttr=True)[0]

                # Get the output geometry data as MObject
                sel = self.om.MSelectionList()
                sel.add(mesh)
                # dep = self.om.MObject()
                dep = sel.getDependNode(0)
                fn_dep = self.om.MFnDependencyNode(dep)
                plug = fn_dep.findPlug(out_attr, True)
                obj = plug.asMObject()

                # Use the MFnGeometryData class to query the components for a tag
                # expression
                fn_geodata = self.om.MFnGeometryData(obj)

                # Components MObject
                components = fn_geodata.resolveComponentTagExpression(tag)
                dag_path = self.om.MDagPath.getAPathTo(dep)
                fn_mesh = self.om.MFnMesh(dag_path)

                return dag_path, components, fn_mesh

            def get_geometry_components(skin_cls):
                """Get the geometry components from skincluster
                Arguments:
                    skin_cls (PyNode): The skincluster node
                Returns:
                    dag_path: The dagpath for the components
                    componets: The skincluster componets
                """
                # Brute force to try the old method using deformerSet. If fail will try
                # to use Maya 2022 component tag expression
                try:
                    fnSet = self.om.MFnSet(skin_cls.__apimfn__().deformerSet())
                    members = self.om.MSelectionList()
                    fnSet.getMembers(members, False)
                    dag_path = self.om.MDagPath()
                    components = self.om.MObject()
                    members.getDagPath(0, dag_path, components)
                    fn_mesh = self.om.MFnMesh(dag_path)

                    return dag_path, components, fn_mesh
                except:
                    return get_mesh_components_from_tag_expression(skin_cls)

            def collect_data(skin_cls):
                dag_path, _, fn_mesh = get_geometry_components(skin_cls)
                space = self.om.MSpace.kObject
                vertex_count = fn_mesh.numVertices
                positions: np.ndarray[float, Any] = np.empty((vertex_count, 3), dtype=float)
                for index in range(vertex_count):
                    position = fn_mesh.getPoint(index, space)
                    positions[index] = np.array([position.x, position.y, position.z], dtype=float)

                weights = skin_cls.getWeights(dag_path)
                weights = np.array([np.array(weight, dtype=float) for weight in weights], dtype=float)
                return weights, positions

            skin_cluster = get_skin_cluster(_obj)
            if skin_cluster is None:
                raise RuntimeError("No skin cluster found!")

            return collect_data(skin_cluster)

        @timer(get_timer_dict)
        def skin_plus_plus_get_skin_data(_obj: str):
            return skin_plus_plus.get_skin_data(_obj)


        get_function_list = (
            cmds_get_skin_weights,
            pymel_get_skin_weights,
            skin_plus_plus_get_skin_data,
        )

        obj = "test_mesh_high"
        self.run_functions(get_function_list, obj)
        self.process_results(get_timer_dict)

    def test_set_performance(self):

        set_timer_dict: dict[str, tuple[float, Any, str]] = {}

        @timer(set_timer_dict)
        def set_skin_weights_om(_obj: str, _bone_ids: tuple[tuple[int,...]], _weights: tuple[tuple[float,...]]):
            # Get API handles
            selection_list = self.om.MSelectionList()
            selection_list.add(_obj)
            dag, components = selection_list.getComponent(0)
            fn_skin = self.oman.MFnSkinCluster(selection_list.getDependNode(1))
            fn_mesh = self.om.MFnMesh(dag)

            influences = [item for sublist in _bone_ids for item in sublist]
            influences.sort()
            influences_set = set(influences)
            # Set skin weights
            influences = self.om.MIntArray()
            for influence in influences_set:
                influences.append(influence)

            vertex_count = fn_mesh.numVertices
            array_count = len(influences * vertex_count)
            weights = self.om.MDoubleArray(array_count, 0)

            # Tried to match your for loop setup (first iterate vertices, then influences)
            for vertex_index, vertex_weights in _weights:
                for influence_index, influence in enumerate(influences):
                    weights_index = (len(influences) * vertex_index) + influence_index
                    weights[weights_index] = 0.2 # Set some random weight for now

            fn_skin.setWeights(dag, components, influences, weights, normalize=True)

        @timer(set_timer_dict)
        def set_skin_weights(_obj, _bone_ids, _weights):
            skin_plus_plus.set_skin_weights(
                _obj.Name,
                _bone_ids,
                _weights
            )

        set_function_list = (
            set_skin_weights_om,
            set_skin_weights,
        )

        obj = self.cmds.GetNodeByName("test_mesh_high")
        self.run_functions(set_function_list, obj)
        self.process_results(set_timer_dict)


def get_sorted_indicies(indicies: list[list[int]], old_names: list[str], new_names: list[str]):
    name_mapping = {old_name: new_index for new_index, old_name in enumerate(new_names)}

    sorted_bone_ids: list[list[int]] = []
    for index, vert_bone_ids in enumerate(indicies):
        sorted_bone_ids[index] = [name_mapping[old_names[bone_id]] for bone_id in vert_bone_ids]


def add_bones():

    from maya import cmds
    from maya import mel

    import maya.OpenMaya as om
    import maya.OpenMayaAnim as oman
    import pymel.core as pm

    selection_list = om.MSelectionList()
    # status = om.MStatus()
    mobject = om.MObject()
    print(mobject)
    fnInfluence = oman.MFnIkJoint(mobject);

if __name__ == "__main__":
    pass
    skin_plus_plus.io.max_to_maya(file_type=skin_plus_plus.FileType.json)
    # skin_plus_plus
    # bones = ["one", "two"]
    # ids = np.array([[0, 1], [1, 0]], dtype=np.float64)
    # weights = np.array([[0.25, 0.75], [0.25, 0.75]], dtype=np.float64)
    # pos = np.array([[0.1, 0.75, 2.0], [0.25, 0.75, 30]], dtype=np.float64)
    # sd = skin_plus_plus.skin_plus_plus_py.SkinData(bones, ids, weights, pos)
    # print(sd)

    # print(sd.bone_ids)
    # skin_plus_plus.io.save(file_type=skin_plus_plus.FileType.json)
    # skin_plus_plus.io.load(file_type=skin_plus_plus.FileType.json)
    # import json
    # import pickle

    # path = r"C:\Users\Sheaky\Documents\3ds Max 2023\scenes\_Data\Sphere001_GEO.skpp-json"

    # with open(path, "r") as file:
    #     data = json.load(file)
    #     skin_data = skin_plus_plus.SkinData(
    #         tuple(data["bone_names"]),
    #         tuple(data["bone_ids"]),
    #         tuple(data["weights"]),
    #         tuple(data["positions"])
    #     )
    # with open(path, "rb") as file:
    #     skin_data = pickle.load(file)

    # # print(skin_data)
    # # # for ids in skin_data.bone_ids:
    # # #     print(ids)
    # skin_plus_plus.set_skin_weights("Sphere001", skin_data)
    # sd = skin_plus_plus.get_skin_data("Sphere001")
    # print(sd.weights)
    # for index, weights in enumerate(sd.weights):
    #     print(f"{index}: {weights}")
    # # skin_data = skin_plus_plus.get_skin_data("Weapon_Shield_1H_001_Model_Main_01_:Shield_GEO")
    # # print(skin_data)
    # # skin_plus_plus.set_skin_weights("SM_EliteEnemy_Axe_GEO", skin_data)
    # # print(skin_data.weights[2])
    # # print(skin_data.bone_ids[0])
