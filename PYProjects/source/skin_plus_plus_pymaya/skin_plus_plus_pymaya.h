#pragma once
#include <skin_plus_plus_py.h>

#include <maya/MDoubleArray.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
//#include <maya/MItCurveCV.h>
//#include <maya/MItMeshVertex.h>
//#include <maya/MItSurfaceCV.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MPlug.h>
#include <maya/MSelectionList.h>



const char* convertStringToChar(std::string text)
{
	const char* charArray;
	charArray = &text[0];
	return charArray;
}


bool getDagPathAndComponent(MString name, MDagPath& dagPath, MObject& component)
{
	MObject node;
	MSelectionList selectionList;
	selectionList.clear();
	selectionList.add(name, true);
	if (selectionList.isEmpty())
	{
		return false;
	}
	for (size_t i = 0; i < selectionList.length(); i++)
	{
		MStatus status = selectionList.getDependNode(i, node);
		if (status == MS::kSuccess) // && node.hasFn(MFn::kSkinClusterFilter))
		{
			selectionList.getDagPath(i, dagPath, component);
			if (component.isNull())
			{
				//Try to get the skin from the sel;
				MStatus status;
				dagPath.extendToShape();

				MFnSingleIndexedComponent singleIndexComponent;
				singleIndexComponent.setComplete(true);
				component = singleIndexComponent.create(MFn::kMeshVertComponent, &status);
				if (status != MS::kSuccess || component.isNull()) py::print("Component not defined!");
			}
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


MStatus getMeshAndSkinFns(MFnMesh& fnMesh, MFnSkinCluster& fnSkinCluster)
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
		throw std::exception("Given node is invalid or is not compatible with skin clusters");
	}

	return fnSkinCluster.setObject(itDependencyGraph.currentItem());
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

	//MFnMesh* fnMesh;
	MFnMesh fnMesh;

	MFnSkinCluster fnSkinCluster;

	MDagPath dagPath;

	MObject component;

	MString name;

public:
	SkinManagerMaya(const wchar_t* name) {
		this->initialise(name);
	}
	//~SkinManagerMaya() { delete this->fnMesh; }
	~SkinManagerMaya() {}

	// Initialise the skin manager with the given node name
	bool initialise(const wchar_t* name);

	// Get the skin weights from the given node's skin modifier
	//std::vector<std::vector<std::vector <float>>> getSkinWeights();

	// Get the vertex weights, bone ids and positions from the given node
	PySkinData getData();

	// Set the skin weights to the given node's skin modifier
	bool setSkinWeights(eg::MatrixXi& boneIDs, eg::MatrixXf& vertexWeights);
};