#include <skin_plus_plus_pymaya.h>




//bool getMeshPositions(const MDagPath& dagPath, Array<Vector3>* pointArray)
//{
//	nvDebugCheck(pointArray != NULL);
//
//	MStatus status;
//	MFnMesh fnMesh(dagPath, &status);
//
//	MItMeshPolygon polyIt(dagPath, MObject::kNullObj, &status);
//	if (MS::kSuccess != status) return false;
//
//	// Add positions.
//	MPointArray positionArray;
//	status = fnMesh.getPoints(positionArray, MSpace::kObject);
//	if (MS::kSuccess != status) return false;
//
//	const uint positionCount = positionArray.length();
//	pointArray->reserve(positionCount);
//
//	for (uint i = 0; i < positionCount; i++)
//	{
//		MPoint point = positionArray[i];
//		pointArray->append(Vector3(point.x, point.y, point.z));
//	}
//
//	return true;
//}


bool getDagPathAndComponent(MString name, MDagPath& dagPath, MObject& component)
{
	MObject node;
	MSelectionList selectionList;
	selectionList.clear();
	//py::print(name.asChar());
	selectionList.add(name, true);
	if (selectionList.isEmpty())
	{
		return false;
	}
	for (size_t i = 0; i < selectionList.length(); i++)
	{
		MStatus status = selectionList.getDependNode(0, node);
		if (status == MS::kSuccess) // && node.hasFn(MFn::kSkinClusterFilter))
		{
			selectionList.getDagPath(0, dagPath, component);
			return dagPath.isValid();
		}

	}
	//auto exceptionString = fmt::format("Given node is invalid or is not compatible with skin clusters: {}!", name);
	//const char* exceptionChar = exceptionString.c_str();
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


void SkinManagerMaya::collectWeightsAndBoneIDs(
	PySkinData* pySkinData,
	MDoubleArray weights,
	unsigned influenceCount,
	unsigned vertexIndex
)
{
	//if (influenceCount > this->maximumVertexWeightCount)
	//{
	//	pySkinData->setMaximumVertexWeightCount(influenceCount);
	//	this->maximumVertexWeightCount = influenceCount;
	//}

	//for (size_t boneIndex = 0; boneIndex < influenceCount; boneIndex++)
	//{
	//	for (size_t vertexWeightIndex = 0; vertexWeightIndex < vertexWeightCount; vertexWeightIndex++)
	//	{
	//		auto influenceWeight = weights[boneIndex * vertexWeightIndex * vertexWeightCount];
	//		if (influenceWeight <= 0) continue;
	//		pySkinData->weights(vertexIndex, boneIndex) = influenceWeight;
	//		pySkinData->boneIDs(vertexIndex, boneIndex) = boneIndex;
	//	}
	//}
	//for (size_t influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
	//{
	//	auto influenceWeight = weights[influenceIndex * vertexIndex];
	//	if (influenceWeight <= 0) continue;
	//	pySkinData->weights(vertexIndex, influenceIndex) = influenceWeight;
	//	pySkinData->boneIDs(vertexIndex, influenceIndex) = influenceIndex;
	//}
}


bool SkinManagerMaya::initialise(const wchar_t* name)
{
	MStatus status;
	if (getDagPathAndComponent(name, this->dagPath, this->component) == false) return false;
	MFnMesh fnMesh1(this->dagPath, &status);
	MObject meshObj = fnMesh1.object(&status);
	this->fnMesh.setObject(meshObj);
	//this->fnMesh = new MFnMesh(this->dagPath, &status);
	status = getMeshAndSkinFns(fnMesh, fnSkinCluster);
	if (status != MS::kSuccess)
	{
		this->isValid = false;
		return false;
	}
	py::print("success!");

	this->isValid = true;
	return true;
};


PySkinData SkinManagerMaya::getData()
{
	if (!this->isValid)
	{
		throw std::exception("1.SkinData object is invalid. Cannot get skin weights.");
	}
	auto vertexCount = this->fnMesh.numVertices();
	py::print(fmt::format("vertexCount: {}", vertexCount));
	if (vertexCount == 0)
	{
		throw std::exception("Mesh has no vertices!");
	}
	MStatus status;
	if (status != MS::kSuccess)
	{
		throw std::exception("2.SkinData object is invalid. Cannot get skin weights.");
	}
	MDoubleArray weights;
	unsigned boneCount;
	this->fnSkinCluster.getWeights(this->dagPath, this->component, weights, boneCount);
	PySkinData pySkinData = PySkinData(vertexCount, this->maximumVertexWeightCount);

	MDagPathArray dagPathArray;
	this->fnSkinCluster.influenceObjects(dagPathArray, &status);
	std::vector<const char*> boneNames(dagPathArray.length());
	for (size_t i = 0; i < dagPathArray.length(); i++)
	{
		boneNames[i] = dagPathArray[i].fullPathName().asChar();
	}
	MPoint mPoint;
	pySkinData.setMaximumVertexWeightCount(boneCount);
	for (size_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		size_t influenceIndex = 0;
		unsigned baseWeightIndex = vertexIndex * boneCount;
		for (size_t boneIndex = 0; boneIndex < boneCount; boneIndex++)
		{
			unsigned weightIndex = baseWeightIndex + boneIndex;
			double influenceWeight = weights[weightIndex];
			pySkinData.weights(vertexIndex, influenceIndex) = influenceWeight;
			pySkinData.boneIDs(vertexIndex, influenceIndex) = boneIndex;
			influenceIndex += 1;
		}
		fnMesh.getPoint(vertexIndex, mPoint, MSpace::kObject);
		pySkinData.positions(vertexIndex, 0) = mPoint.x;
		pySkinData.positions(vertexIndex, 1) = mPoint.y;
		pySkinData.positions(vertexIndex, 2) = mPoint.z;
	}

	return pySkinData;
}


PYBIND11_MODULE(skin_plus_plus_pymaya, m) {
	m.def("get_data", [&](wchar_t* name)
		{
			SkinManagerMaya skinData(name);
			PySkinData pySkinData = skinData.getData();
			return pySkinData;

		},
		"Get Skin Data",
		py::arg("name")
	);
	m.def("get_vertex_positions", [&](wchar_t* name)
		{
			SkinManagerMaya skinData(name);
			//PySkinData* pySkinData = skinData.getData();
			//return pySkinData;

		},
		"Get Skin Data",
		py::arg("name")
	);
	m.def("set_skin_weights", [&](wchar_t* name, PySkinData& skinData)
		{
			SkinManagerMaya skinManager(name);
			//return skinManager.setSkinWeights(skinData.boneIDs, skinData.weights);
		},
		"Set Skin Weights",
		py::arg("name"),
		py::arg("skin_data")
	);
}