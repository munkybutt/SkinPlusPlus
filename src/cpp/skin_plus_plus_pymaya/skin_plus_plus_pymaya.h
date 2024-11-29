#pragma once
#include <skin_plus_plus_py.h>

#include <maya/MDagModifier.h>
#include <maya/MDoubleArray.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnSet.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MMatrixArray.h>
#include <maya/MPlug.h>
#include <maya/MQuaternion.h>
#include <maya/MSelectionList.h>

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnMatrixAttribute.h>


bool getDagPathAndComponent(MString name, MDagPath& dagPath, MObject& component, std::optional<VertexIDsMatrix> vertexIDs=std::nullopt)
{
	MStatus status;
	MObject node;
	MSelectionList selectionList;
	selectionList.clear();
	selectionList.add(name, true);
	if (selectionList.isEmpty())
	{
		return false;
	}
	for (UINT i = 0; i < selectionList.length(); i++)
    {
        status = selectionList.getDependNode(i, node);
		if (status != MS::kSuccess) // && node.hasFn(MFn::kSkinClusterFilter))
		{
			continue;
		}
        selectionList.getDagPath(i, dagPath);
        if (component.isNull())
        {
            dagPath.extendToShape();

            MFnSingleIndexedComponent singleIndexComponent;
			if (vertexIDs.has_value())
			{
				const VertexIDsMatrix vertexIDs_ = vertexIDs.value();
				MIntArray vertexArray;
				vertexArray.setLength(vertexIDs_.size());
				for (UINT i = 0; i < vertexIDs_.size(); i++)
				{
					vertexArray[i] = vertexIDs_[i];
				}
				singleIndexComponent.addElements(vertexArray);
			}
            singleIndexComponent.setComplete(true);
            component = singleIndexComponent.create(MFn::kMeshVertComponent, &status);
            if (status != MS::kSuccess || component.isNull()) py::print("Component not defined!");
        }


        return dagPath.isValid();

    }
    throw std::exception("Given node is invalid or is not compatible with skin clusters");
}


bool getDagPathAndComponent(const wchar_t* name, MDagPath& dagPath, MObject& component)
{
	MString mName(name);
	return getDagPathAndComponent(mName, dagPath, component);
}


MDagPathArray getNodesFromNames(MStringArray names)
{
	MStatus status;
	MObject node;
	MDagPathArray dagPaths;
	MSelectionList selectionList;
	selectionList.clear();
	for (MString name: names)
	{
		selectionList.add(name, true);
	}
	if (selectionList.isEmpty())
	{
		return dagPaths;
	}
	for (UINT i = 0; i < selectionList.length(); i++)
	{
		status = selectionList.getDependNode(i, node);
		if (status == MS::kSuccess) // && node.hasFn(MFn::kSkinClusterFilter))
		{
			MDagPath dagPath;
			MObject component;
			selectionList.getDagPath(i, dagPath, component);
			if (component.isNull())
			{
				//Try to get the skin from the sel:
				dagPath.extendToShape();
				MFnSingleIndexedComponent singleIndexComponent;
				singleIndexComponent.setComplete(true);
				component = singleIndexComponent.create(MFn::kMeshVertComponent, &status);
				if (status != MS::kSuccess || component.isNull()) py::print("Component not defined!");
			}

			if (dagPath.isValid())
			{
				dagPaths.append(dagPath);
			}
		}
	}
	return dagPaths;
}


bool isSkinClusterValid(MObject& meshNode, bool throwExceptions = true)
{
    MStatus status;
    MFnDependencyNode meshDepNode(meshNode);
    MPlug skinClusterPlug = meshDepNode.findPlug("inMesh", false, &status);

    if (!status)
    {
        auto message = fmt::format("The node: '{}' is not connected to a skin cluster!", meshDepNode.name().asChar());
        if (throwExceptions)
        {
            throw std::exception(message.c_str());
        }
        return false;
    }

    MPlugArray skinClusterConnections;
    skinClusterPlug.connectedTo(skinClusterConnections, true, false, &status);
    if (!status || skinClusterConnections.length() > 1)
    {
        auto message = fmt::format("The node: '{}' is connected to multiple skin clusters: ", meshDepNode.name().asChar());
        std::vector<MString> skinClusterNames;
        for (UINT i = 0; i < skinClusterConnections.length(); ++i)
        {
            MObject skinClusterNode = skinClusterConnections[i].node();
            MFnSkinCluster skinClusterFn(skinClusterNode);
            skinClusterNames.push_back(skinClusterFn.name());
            message += fmt::format("{}{}", skinClusterNames[i].asChar(), (i < skinClusterNames.size() - 1) ? ", " : ".");
        }
        if (throwExceptions)
        {
            throw std::exception(message.c_str());
        }
        return false;
    }
    return true;
}


MStatus getMeshAndSkinFns(MFnMesh& fnMesh, MFnSkinCluster& fnSkinCluster, MString name)
{
	MStatus status;
	MObject meshObj = fnMesh.object(&status);
	if (status != MS::kSuccess)
	{
		return status;
	}
	MItDependencyGraph itDependencyGraph(
		meshObj,
		MFn::kSkinClusterFilter,
		MItDependencyGraph::kUpstream,
		MItDependencyGraph::kBreadthFirst,
		MItDependencyGraph::kNodeLevel,
		&status
	);
	if (status != MS::kSuccess)
	{
		return status;
	}
	if (itDependencyGraph.isDone())
	{
        auto message = fmt::format("Given node: '{}' is invalid or is not compatible with skin clusters!", name.asChar());
		throw std::exception(message.c_str());
	}
    //return fnSkinCluster.setObject(itDependencyGraph.currentItem());
    
    auto isSkinClusterValidResult = isSkinClusterValid(meshObj, true);
    if (isSkinClusterValidResult)
    {
        return fnSkinCluster.setObject(itDependencyGraph.currentItem());
    }

    return MS::kFailure;
}


class SkinManagerMaya
{
private:
	// Whether or not the skin manager is in a valid state.
	// Either no node with the given name can be found, or
	// the given node has no skin modifier on it.
	bool isValid = false;

	// Used to track the maximum number of vertex weights, allowing data to be resized only when needed
	int maximumVertexWeightCount;

	// Get the vertex weights and bone ids and add them to the given PySkinData
	void collectWeightsAndBoneIDs(PySkinData* pySkinData, MDoubleArray weights, unsigned influenceCount, unsigned vertexIndex);

	UINT vertexCount;

	MFnMesh fnMesh;

	MFnSkinCluster fnSkinCluster;

	MDagPath dagPath;

	MObject component;

	MString name;

public:
	SkinManagerMaya(const wchar_t* name, std::optional<VertexIDsMatrix> vertexIDs = std::nullopt) {
		this->initialise(name, vertexIDs=vertexIDs);
	}
	//~SkinManagerMaya() { delete this->fnMesh; }
	~SkinManagerMaya() {}

	// Initialise the skin manager with the given node name and optional list of vertices to query.
	bool initialise(const wchar_t* name, std::optional<VertexIDsMatrix> vertexIDs = std::nullopt);

	// Get the skin weights from the given node's skin modifier
	//std::vector<std::vector<std::vector <float>>> getSkinWeights();
	MObject addMissingBones(std::vector<std::wstring>& missingBoneNames, const UINT& skinnedBoneCount);

	// Get the vertex weights, bone ids and positions from the given node
	PySkinData extractSkinData(const bool safeMode = true);

	// Set the skin weights to the given node's skin modifier
	bool applySkinData(PySkinData& skinData);
};
