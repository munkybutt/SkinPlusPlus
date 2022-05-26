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


bool SkinManagerMaya::initialise(const wchar_t* name)
{
	MStatus status;
	this->name = MString(name);
	if (getDagPathAndComponent(name, this->dagPath, this->component) == false) return false;
	MFnMesh fnMesh1(this->dagPath, &status);
	MObject meshObj = fnMesh1.object(&status);
	this->fnMesh.setObject(meshObj);
	//this->fnMesh = new MFnMesh(this->dagPath, &status);
	status = getMeshAndSkinFns(this->fnMesh, this->fnSkinCluster);
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
		throw std::exception("SkinData object is invalid. Cannot get skin weights.");
	}
	auto vertexCount = this->fnMesh.numVertices();
	if (vertexCount == 0)
	{
		throw std::exception("Mesh has no vertices!");
	}
	MDoubleArray weights;
	unsigned boneCount;
	this->fnSkinCluster.getWeights(this->dagPath, this->component, weights, boneCount);
	PySkinData pySkinData = PySkinData(vertexCount, this->maximumVertexWeightCount);

	MDagPathArray dagPathArray;
	MStatus status;
	this->fnSkinCluster.influenceObjects(dagPathArray, &status);
	if (status != MS::kSuccess)
	{
		throw std::exception("Failed to find influence objects!");
	}
	std::vector<const char*> boneNames(dagPathArray.length());
	pySkinData.boneNames = std::vector<std::string>(dagPathArray.length());
	for (size_t boneIndex = 0; boneIndex < dagPathArray.length(); boneIndex++)
	{
		pySkinData.boneNames[boneIndex] = fmt::format("{}", dagPathArray[boneIndex].partialPathName().asChar());
	}
	MPoint mPoint;
	pySkinData.setMaximumVertexWeightCount(boneCount);
	for (size_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		size_t influenceIndex = 0;
		size_t weightIndex = vertexIndex * boneCount;
		for (size_t boneIndex = 0; boneIndex < boneCount; boneIndex++)
		{
			weightIndex += boneIndex;
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


bool SkinManagerMaya::setSkinWeights(eg::MatrixXi& boneIDs, eg::MatrixXf& vertexWeights)
{
	size_t vertexCount = boneIDs.rows();
	size_t vertexWeightsRows = vertexWeights.rows();
	size_t influenceCount = boneIDs.cols();
	size_t vertexWeightsCols = vertexWeights.cols();
	if (vertexCount != vertexWeightsRows)
	{
		const char* exceptionText = convertStringToChar(
			fmt::format("boneIDs row size: {} does not match vertexWeights row size: {}", vertexCount, vertexWeightsRows)
		);
		throw std::length_error(exceptionText);
	}
	if (influenceCount != vertexWeightsCols)
	{
		//fmt::to_char();
		const char* exceptionText = convertStringToChar(
			fmt::format("boneIDs column size: {} does not match vertexWeights column size: {}", influenceCount, vertexWeightsCols)
		);
		throw std::length_error(exceptionText);
	}
	size_t arraySize = vertexCount * influenceCount;
	MIntArray mBoneIDs(influenceCount);
	MDoubleArray mWeights(arraySize);
	for (size_t influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++) mBoneIDs[influenceIndex] = influenceIndex;
	// Unpack nested arrays like so: [[0, 1], [2, 3]] -> [0, 1, 2, 3]
	for (size_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		size_t arrayIndex = vertexIndex * influenceCount;
		for (size_t influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
		{
			arrayIndex += influenceIndex;
			auto boneID = boneIDs(vertexIndex, influenceIndex);
			auto vertexWeight = vertexWeights(vertexIndex, influenceIndex);
			mWeights[arrayIndex] = vertexWeight;
		}
	}

	MStatus status = this->fnSkinCluster.setWeights(
		this->dagPath,
		this->component,
		mBoneIDs,
		mWeights,
		true
		//MDoubleArray * oldValues = NULL
	);
	if (status != MS::kSuccess) return false;
	py::print("Applied Skin Data Successfully!");
	return true;
}


PYBIND11_MODULE(skin_plus_plus_pymaya, m) {
	// This makes the base SkinData class available to the module:
	#include <skin_plus_plus_py.h>

	m.def("get_skin_data", [&](wchar_t* name)
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
			PySkinData pySkinData = skinData.getData();
			return pySkinData.positions;

		},
		"Get Vertex Positions",
		py::arg("name")
	);
	m.def("set_skin_weights", [&](wchar_t* name, PySkinData& skinData)
		{
			SkinManagerMaya skinManager(name);
			return skinManager.setSkinWeights(skinData.boneIDs, skinData.weights);
		},
		"Set Skin Weights",
		py::arg("name"),
		py::arg("skin_data")
	);
}