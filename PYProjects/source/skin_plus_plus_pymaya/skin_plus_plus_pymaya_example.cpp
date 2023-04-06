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

#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MObject.h>


//MStatus getNodesByName(const MStringArray& nodeNames, MObject& node)
//{
//	MSelectionList selectionList;
//	MStatus status = MGlobal::getSelectionListByName(nodeNames, selectionList);
//	if (status == MS::kSuccess)
//	{
//		status = selectionList.getDependNode(0, node);
//	}
//	return status;
//}


bool SkinManagerMaya::initialise(const wchar_t* name)
{
	MStatus status;
	this->name = MString(name);
	if (getDagPathAndComponent(name, this->dagPath, this->component) == false)
	{
		return false;
	}
	MFnMesh fnMesh1(this->dagPath, &status);
	MObject meshObj = fnMesh1.object(&status);
	this->fnMesh.setObject(meshObj);
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

void removeBonesFromBindPose(MPlug bindPoseMatrixArrayPlug, MPlug bindPoseMemberArrayPlug, MPlugArray connectedPlugs, MDGModifier& dgModifier)
{
	MStatus status;
	for (unsigned i = 0; i < bindPoseMatrixArrayPlug.numConnectedElements(); i++)
	{
		MPlug matrixPlug = bindPoseMatrixArrayPlug.connectionByPhysicalIndex(i, &status);
		matrixPlug.connectedTo(connectedPlugs, true, false);
		if (connectedPlugs.length())
		{
			dgModifier.disconnect(connectedPlugs[0], matrixPlug);
		}
	}
	for (unsigned i = 0; i < bindPoseMemberArrayPlug.numConnectedElements(); i++)
	{
		MPlug memberPlug = bindPoseMemberArrayPlug.connectionByPhysicalIndex(i, &status);
		memberPlug.connectedTo(connectedPlugs, true, false);
		if (connectedPlugs.length())
		{
			dgModifier.disconnect(connectedPlugs[0], memberPlug);
		}
	}
}


void SkinManagerMaya::addMissingBones(std::vector<std::string> missingBoneNames, long currentBoneCount)
{
	long newBoneCount = currentBoneCount;
	MStatus status;
	MStringArray boneNames;
	boneNames.setLength(missingBoneNames.size());
	for (size_t i = 0; i < missingBoneNames.size(); i++)
	{
		boneNames[i] = MString(missingBoneNames[i].c_str());
	}
	auto missingBones = getNodesFromNames(boneNames);

	MPlugArray connectedPlugs;

	bool bindPoseExists = true;
	MPlug bindPlug = this->fnSkinCluster.findPlug("bindPose", false, &status);
	if (status == MStatus::kFailure)
	{
		bool bindPoseExists = false;
		//auto errorMessage = fmt::format("Failed to find bind pose: {}!", this->fnSkinCluster.name());
		//throw std::exception(errorMessage.c_str());
	}
	if (!bindPlug.connectedTo(connectedPlugs, true, false))
	{
		auto errorMessage = fmt::format("Failed to find bind pose: {}!", this->fnSkinCluster.name());
		throw std::exception(errorMessage.c_str());
	}
	MFnDependencyNode fnBindPose(connectedPlugs[0].node());
	if (fnBindPose.typeName() != "dagPose")
	{
		auto errorMessage = fmt::format("Invalid bindPose: {}!", fnBindPose.name());
		throw std::exception(errorMessage.c_str());
	}

	MPlug bindPoseMatrixArrayPlug = fnBindPose.findPlug("worldMatrix", false, &status);
	if (status == MStatus::kFailure)
	{
		auto errorMessage = fmt::format("Failed to get worldMatrix plug: {}!", fnBindPose.name());
		throw std::exception(errorMessage.c_str());
	}
	MPlug bindPoseMemberArrayPlug = fnBindPose.findPlug("members", false, &status);
	if (status == MStatus::kFailure)
	{
		auto errorMessage = fmt::format("Failed to get members plug: {}!", fnBindPose.name());
		throw std::exception(errorMessage.c_str());
	}

	MDGModifier dgModifier;
	MMatrixArray ignoredPreMatrices;
	MPlug matrixArrayPlug = this->fnSkinCluster.findPlug("matrix", false, &status);
	MPlug bindPreMatrixArrayPlug = this->fnSkinCluster.findPlug("bindPreMatrix", false, &status);

	MSelectionList missingBoneSelectionList;
	for (MDagPath bone : missingBones)
	{
		missingBoneSelectionList.add(bone);
	}
	MObject mObject;
	MDagPath dagPath;
	for (unsigned i = 0; i < matrixArrayPlug.numConnectedElements(); i++)
	{
		MPlug matrixPlug = matrixArrayPlug.connectionByPhysicalIndex(i, &status);
		matrixPlug.connectedTo(connectedPlugs, true, false);
		if (!connectedPlugs.length())
		{
			continue;
		}
		
		MFnIkJoint fnIkJoint(connectedPlugs[0].node());
		fnIkJoint.getPath(dagPath);
		
		if (!missingBoneSelectionList.hasItem(dagPath))
		{
			MPlug preMatrixPlug = bindPreMatrixArrayPlug.elementByLogicalIndex(i);
			preMatrixPlug.getValue(mObject);
			MFnMatrixData fnMatrixData(mObject);
			ignoredPreMatrices.append(fnMatrixData.matrix());
			//ignoreInfluence.push_back(false);
			//indexMap.push_back(missingBoneSelectionList.length());
			missingBoneSelectionList.add(connectedPlugs[0].node());
			newBoneCount++;
		}
		dgModifier.disconnect(connectedPlugs[0], matrixPlug);
	}

	MPlug lockWeightsArrayPlug = this->fnSkinCluster.findPlug("lockWeights", false, &status);
	for (unsigned i = 0; i < lockWeightsArrayPlug.numConnectedElements(); i++)
	{
		MPlug lockWeightsPlug = lockWeightsArrayPlug.connectionByPhysicalIndex(i, &status);
		lockWeightsPlug.connectedTo(connectedPlugs, true, false);
		if (connectedPlugs.length())
		{
			dgModifier.disconnect(connectedPlugs[0], lockWeightsPlug);
		}
	}
	MPlug paintPlug = this->fnSkinCluster.findPlug("paintTrans", false, &status);
	paintPlug.connectedTo(connectedPlugs, true, false);
	if (connectedPlugs.length())
	{
		dgModifier.disconnect(connectedPlugs[0], paintPlug);
	}

	if (bindPoseExists)
	{
		removeBonesFromBindPose(bindPoseMatrixArrayPlug, bindPoseMemberArrayPlug, connectedPlugs, dgModifier);
	}

	if (!dgModifier.doIt())
	{
		dgModifier.undoIt();
		throw std::exception("Failed to reset bone connections!");
	}


	// make connections from influences to skinCluster and bindPose
	for (size_t i = 0; i < missingBoneSelectionList.length(); i++)
	{
		//if (ignoreInfluence[i])
		//{
		//	continue;
		//}

		//int index = indexMap[i];
		int newBoneIndex = currentBoneCount + i;
		status = missingBoneSelectionList.getDependNode(i, mObject);
		MFnIkJoint fnInfluence(mObject, &status);
		MPlug influenceMatrixPlug = fnInfluence.findPlug("worldMatrix", false, &status).elementByLogicalIndex(0, &status);
		MPlug influenceMessagePlug = fnInfluence.findPlug("message", false, &status);
		MPlug influenceBindPosePlug = fnInfluence.findPlug("bindPose", false, &status);
		MPlug influenceLockPlug = fnInfluence.findPlug("lockInfluenceWeights", false, &status);
		if (!status)
		{
			// add the lockInfluenceWeights attribute if it doesn't exist
			MFnNumericAttribute fnNumericAttribute;
			MObject attribute = fnNumericAttribute.create("lockInfluenceWeights", "liw", MFnNumericData::kBoolean, false);
			fnInfluence.addAttribute(attribute);
			influenceLockPlug = fnInfluence.findPlug("lockInfluenceWeights", false, &status);
		}

		// connect influence to the skinCluster
		MPlug matrixPlug = matrixArrayPlug.elementByLogicalIndex(newBoneIndex);
		MPlug lockPlug = lockWeightsArrayPlug.elementByLogicalIndex(newBoneIndex);
		dgModifier.connect(influenceMatrixPlug, matrixPlug);
		dgModifier.connect(influenceLockPlug, lockPlug);

		// connect influence to the bindPose
		if (!ignoreBindPose)
		{
			MPlug bindPoseMatrixPlug = bindPoseMatrixArrayPlug.elementByLogicalIndex(newBoneIndex);
			MPlug memberPlug = bindPoseMemberArrayPlug.elementByLogicalIndex(newBoneIndex);
			dgModifier.connect(influenceMessagePlug, bindPoseMatrixPlug);
			dgModifier.connect(influenceBindPosePlug, memberPlug);
		}
	}

}


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

	MDagPathArray skinnedBones;
	MStatus status;
	this->fnSkinCluster.influenceObjects(skinnedBones, &status);
	if (status != MS::kSuccess)
	{
		throw std::exception("Failed to find influence objects!");
	}
	//std::vector<const char*> boneNames(skinnedBones.length());
	pySkinData.boneNames = std::vector<std::string>(skinnedBones.length());
	for (size_t boneIndex = 0; boneIndex < skinnedBones.length(); boneIndex++)
	{
		pySkinData.boneNames[boneIndex] = fmt::format("{}", skinnedBones[boneIndex].partialPathName().asChar());
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


bool SkinManagerMaya::setSkinWeights(PySkinData& skinData)
{
	size_t vertexCount = skinData.boneIDs.rows();
	size_t vertexWeightsRows = skinData.weights.rows();
	size_t influenceCount = skinData.boneIDs.cols();
	size_t vertexWeightsCols = skinData.weights.cols();
	if (vertexCount != vertexWeightsRows)
	{
		const char* exceptionText = convertStringToChar(
			fmt::format(
				"boneIDs row size: {} does not match vertexWeights row size: {}",
				vertexCount,
				vertexWeightsRows
			)
		);
		throw std::length_error(exceptionText);
	}
	if (influenceCount != vertexWeightsCols)
	{
		const char* exceptionText = convertStringToChar(
			fmt::format(
				"boneIDs column size: {} does not match vertexWeights column size: {}",
				influenceCount,
				vertexWeightsCols
			)
		);
		throw std::length_error(exceptionText);
	}
	
	MDagPathArray skinnedBones;
	MStatus status;
	this->fnSkinCluster.influenceObjects(skinnedBones, &status);
	if (status != MS::kSuccess)
	{
		throw std::exception("Failed to find any bones!");
	}
	auto skinBoneCount = skinnedBones.length();
	auto currentBoneNames = std::vector<std::string>(skinBoneCount);
	for (size_t boneIndex = 0; boneIndex < skinBoneCount; boneIndex++)
	{
		currentBoneNames[boneIndex] = fmt::format("{}", skinnedBones[boneIndex].partialPathName().asChar());
	}
	if (skinBoneCount == 0)
	{
		this->addMissingBones(skinData.boneNames, skinBoneCount);
		this->fnSkinCluster.influenceObjects(skinnedBones, &status);
	}
	auto sortedBoneIDs = skinData.getSortedBoneIDs(currentBoneNames);
	if (sortedBoneIDs.unfoundBoneNames.size() != 0)
	{
		this->addMissingBones(sortedBoneIDs.unfoundBoneNames);
		this->fnSkinCluster.influenceObjects(skinnedBones, &status);
	}
	size_t arraySize = vertexCount * influenceCount;
	MIntArray mBoneIDs(influenceCount);
	MDoubleArray mWeights(arraySize);
	for (size_t influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
	{
		mBoneIDs[influenceIndex] = sortedBoneIDs.sortedBoneIDs[influenceIndex];
	}
	// Unpack nested arrays like so: [[0, 1], [2, 3]] -> [0, 1, 2, 3]
	for (size_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		size_t arrayIndex = vertexIndex * influenceCount;
		for (size_t influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
		{
			arrayIndex += influenceIndex;
			auto boneID = skinData.boneIDs(vertexIndex, influenceIndex);
			auto vertexWeight = skinData.weights(vertexIndex, influenceIndex);
			mWeights[arrayIndex] = vertexWeight;
		}
	}

	status = this->fnSkinCluster.setWeights(
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
			return skinManager.setSkinWeights(skinData);
		},
		"Set Skin Weights",
		py::arg("name"),
		py::arg("skin_data")
	);
}