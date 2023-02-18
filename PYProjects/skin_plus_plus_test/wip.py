from __future__ import annotations

import maya.OpenMaya as om  # type: ignore
import maya.OpenMayaAnim as om_anim  # type: ignore
import traceback


def get_skin_cluster(name: str) -> om_anim.MFnSkinCluster:
    selection_list = om.MSelectionList()
    selection_list.add(name)
    mobject = om.MObject()
    selection_list.getDependNode(0, mobject)
    return om_anim.MFnSkinCluster(mobject)


def get_skin_cluster_lock_weights_plug(skin_cluster: om_anim.MFnSkinCluster) -> om.MPlug:
    return skin_cluster.findPlug("lockWeights", False)


def get_skin_cluster_matrix_array_plug(skin_cluster: om_anim.MFnSkinCluster) -> om.MPlug:
    return skin_cluster.findPlug("matrix", False)


def get_skin_cluster_joints_iter(skin_cluster_matrix_array: om.MPlug):
    plug_array = om.MPlugArray()
    for index in range(skin_cluster_matrix_array.numConnectedElements()):
        plug = skin_cluster_matrix_array.connectionByPhysicalIndex(index)
        plug.connectedTo(plug_array, True, False)
        joint = om_anim.MFnIkJoint(plug_array[0].node())
        yield joint


def get_skin_cluster_joint_names_iter(skin_cluster_matrix_array: om.MPlug, full_name: bool = False):
    dag_path = om.MDagPath()
    for joint in get_skin_cluster_joints_iter(skin_cluster_matrix_array):
        joint.getPath(dag_path)
        name = dag_path.fullPathName() if full_name else dag_path.partialPathName()
        yield name


def get_bind_pose(skin_cluster: om_anim.MFnSkinCluster) -> om.MFnDependencyNode:
    bind_pose_plug: om.MPlug = skin_cluster.findPlug("bindPose", False)
    connected_plugs = om.MPlugArray()
    connected = bind_pose_plug.connectedTo(connected_plugs, True, False)
    if not connected:
        raise RuntimeError("bindPose not connected to array")

    bind_pose = om.MFnDependencyNode(connected_plugs[0].node())
    if not bind_pose.typeName() == "dagPose":
        raise RuntimeError("Dependency node not dagPose")

    return bind_pose


def get_bind_pose_members(bind_pose: om.MFnDependencyNode) -> om.MPlug:
    return bind_pose.findPlug("members", False)


def get_bind_pose_world_matrix_array(bind_pose: om.MFnDependencyNode) -> om.MPlug:
    return bind_pose.findPlug("worldMatrix", False)


def get_joint(name: str):
    selection_list = om.MSelectionList()
    selection_list.add(name)
    mobject = om.MObject()
    selection_list.getDependNode(0, mobject)
    if not mobject.hasFn(om.MFn.kJoint):
        raise RuntimeError(f"Node is not a joint: '{name}'")

    return om_anim.MFnIkJoint(mobject)


def get_joint_world_matrix(joint: om_anim.MFnIkJoint) -> om.MPlug:
    try:
        plug = joint.findPlug("worldMatrix", False)
        return plug.elementByLogicalIndex(0)
    except RuntimeError:
        dag_path = om.MDagPath()
        joint.getPath(dag_path)
        print(dag_path.fullPathName())
        raise


def add_joints_to_skin_cluster(skin_cluster: om_anim.MFnSkinCluster, joint_names: list[str]):
    skin_cluster_matrix_array = get_skin_cluster_matrix_array_plug(skin_cluster)
    skin_cluster_lock_weights_plug = get_skin_cluster_lock_weights_plug(skin_cluster)
    skin_cluster_joint_names = set(get_skin_cluster_joint_names_iter(skin_cluster_matrix_array))

    bind_pose = get_bind_pose(skin_cluster)
    bind_pose_members = get_bind_pose_members(bind_pose)
    bind_pose_world_matrix_array = get_bind_pose_world_matrix_array(bind_pose)

    current_joint_count = skin_cluster_matrix_array.numConnectedElements()
    dag_modifier = om.MDagModifier()
    for index, joint_name in enumerate(joint_names, 1):
        if joint_name in skin_cluster_joint_names:
            print(f"Joint alreadty in skin cluster: {joint_name}")
            continue

        new_bone_index = index + current_joint_count
        joint = get_joint(joint_name)
        joint_world_matrix_plug = get_joint_world_matrix(joint)
        joint_lock_influence_weights_plug: om.MPlug = joint.findPlug("lockInfluenceWeights", False)
        joint_message_plug = joint.findPlug("message", False)
        joint_bind_pose_plug = joint.findPlug("bindPose", False)

        skin_cluster_new_matrix_plug: om.MPlug = skin_cluster_matrix_array.elementByLogicalIndex(
            new_bone_index
        )
        skin_cluster_new_lock_weights_plug: om.MPlug = (
            skin_cluster_lock_weights_plug.elementByLogicalIndex(new_bone_index)
        )
        dag_modifier.connect(joint_world_matrix_plug, skin_cluster_new_matrix_plug)
        dag_modifier.connect(joint_lock_influence_weights_plug, skin_cluster_new_lock_weights_plug)

        bind_pose_matrix_plug = bind_pose_world_matrix_array.elementByLogicalIndex(new_bone_index)
        member_plug = bind_pose_members.elementByLogicalIndex(new_bone_index)
        dag_modifier.connect(joint_message_plug, bind_pose_matrix_plug)
        dag_modifier.connect(joint_bind_pose_plug, member_plug)

    dag_modifier.doIt()


skin_cluster = get_skin_cluster("skinCluster2")
add_joints_to_skin_cluster(
    skin_cluster, ["Point009", "Point010", "Point011", "Point012"]
)


# print("done")

# new_bind_pose_index: om.MPlug = bind_pose_members.elementByLogicalIndex()
# bind_pose_world_matrix_array_index: om.MPlug = bind_pose_world_matrix_array.elementByLogicalIndex()

# joint_message_plug = joint.findPlug("message", False)

# MPlug bindPoseMatrixArrayPlug = fn_bind_pose.findPlug("worldMatrix", false, &status)

# MPlug bind_pose_matrix_plug = bindPoseMatrixArrayPlug.elementByLogicalIndex(new_bone_index)


# print(f"joint_message_plug: {joint_message_plug}")


# bind_pose_plug = joint.findPlug("bindPose", False)

# dag_modifier.connect()
