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
#include <maya/MFnSkinCluster.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MPlug.h>
#include <maya/MSelectionList.h>


//#include <iostream>
//
//#include <maya/MItMeshPolygon.h>
//#include <maya/MPointArray.h>
//
//#include <maya/MTime.h>
//#include <maya/MAnimControl.h>


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
	//bool setSkinWeights(eg::MatrixXi& boneIDs, eg::MatrixXf& vertexWeights);
};