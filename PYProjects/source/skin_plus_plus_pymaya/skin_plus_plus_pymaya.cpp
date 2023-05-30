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
	status = getMeshAndSkinFns(this->fnMesh, this->fnSkinCluster, this->name);
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


std::unordered_set<std::string> getBoneNamesSet(MDagPathArray skinnedBones)
{
	std::vector<std::string> skinnedBoneNames(skinnedBones.length());
	for (UINT i = 0; i < skinnedBones.length(); i++)
	{
		skinnedBoneNames[i] = skinnedBones[i].partialPathName().asChar();
	}
	std::unordered_set<std::string> skinnedBoneNamesSet(skinnedBoneNames.begin(), skinnedBoneNames.end());
	return skinnedBoneNamesSet;
}


//void validateBindPose(MObject bindPoseNode, MDagPathArray skinnedBones)
//{
//	MPlug element;
//	MStatus status;
//	MPlugArray connectedPlugs;
//
//	auto bindPose = MFnDependencyNode(bindPoseNode);
//	auto bindPoseMembersArray = bindPose.findPlug("members", false);
//
//	auto boneNames = getBoneNamesSet(skinnedBones);
//
//	for (UINT i = 0; i < bindPoseMembersArray.numElements(); i++)
//    {
//        element = bindPoseMembersArray.elementByPhysicalIndex(i);
//        auto elementName = element.partialName().asChar();
//        if (!boneNames.count(elementName))
//        {
//            break;
//        }
//
//        //py::print(fmt::format("bind joint name: {}", joint.name().asChar()));
//        //MFnIkJoint(node);
//    }
//}


MObject SkinManagerMaya::addMissingBones(std::vector<std::string> missingBoneNames, MDagPathArray skinnedBones)
{
    MStatus status;
    MSelectionList selectionList;
    MObject dependNode;

    long newBoneIndex = skinnedBones.length();
    auto bind_pose_plug = this->fnSkinCluster.findPlug("bindPose", false);
    MPlugArray connectedPlugs;
    bind_pose_plug.connectedTo(connectedPlugs, true, false, &status);
    if (status != MS::kSuccess) throw std::exception("bindPose not connected to array");

    auto bindPoseNode = connectedPlugs[0].node();
    MFnDependencyNode bindPose;
    bindPose.setObject(bindPoseNode);
    if (bindPose.typeName() != "dagPose") throw std::exception("Dependency node not dagPose");

    auto skinClusterMatrixArray = this->fnSkinCluster.findPlug("matrix", false, &status);
    auto skinClusterLockWeightsArray = this->fnSkinCluster.findPlug("lockWeights", false, &status);

    auto bindPoseMembersArray = bindPose.findPlug("members", false);
    auto bindPoseWorldMatrixArray = bindPose.findPlug("worldMatrix", false);
    MDGModifier dagModifier;
    for (UINT i = 0; i < missingBoneNames.size(); i++)
    {
        newBoneIndex += i;
        selectionList.add(missingBoneNames[i].c_str());
        status = selectionList.getDependNode(i, dependNode);
        if (status != MS::kSuccess) throw std::exception("Failed to get depend node!");
        if (!dependNode.hasFn(MFn::kJoint))
        {
            auto exceptionText = convertStringToChar(
                fmt::format("Node is not a joint: {}", missingBoneNames[i])
            );
            throw std::exception(exceptionText);
        }

        MFnIkJoint joint;
        joint.setObject(dependNode);
        auto jointMessage = joint.findPlug("message", false);
        auto jointBindPose = joint.findPlug("bindPose", false);
        auto jointWorldMatrix = joint.findPlug("worldMatrix", false).elementByPhysicalIndex(0);
        auto jointLockInfluenceWeightsArray = joint.findPlug("lockInfluenceWeights", false);

        auto skinClusterMatrixNewElementIndex = skinClusterMatrixArray.elementByLogicalIndex(newBoneIndex);
        auto skinClusterLockWeightsNewElementIndex = (skinClusterLockWeightsArray.elementByLogicalIndex(newBoneIndex));
        auto bindPoseWorldMatrixNewElementIndex = bindPoseWorldMatrixArray.elementByLogicalIndex(newBoneIndex);
        auto bindPoseMemberNewElementIndex = bindPoseMembersArray.elementByLogicalIndex(newBoneIndex);

        dagModifier.connect(jointWorldMatrix, skinClusterMatrixNewElementIndex);
        dagModifier.connect(jointLockInfluenceWeightsArray, skinClusterLockWeightsNewElementIndex);
        dagModifier.connect(jointMessage, bindPoseWorldMatrixNewElementIndex);
        dagModifier.connect(jointBindPose, bindPoseMemberNewElementIndex);

        py::print("added missing bone: {}", missingBoneNames[i]);
    }
    status = dagModifier.doIt();
    if (status != MS::kSuccess)
    {
        throw std::exception("Failed to add missing bones!");
    }
    return bindPoseNode;
}


PySkinData SkinManagerMaya::getData()
{
    if (!this->isValid)
    {
        throw std::exception("SkinData object is invalid. Cannot get skin weights.");
    }
    UINT vertexCount = this->fnMesh.numVertices();
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
    pySkinData.boneNames = std::vector<std::string>(skinnedBones.length());
    for (UINT boneIndex = 0; boneIndex < skinnedBones.length(); boneIndex++)
    {
        pySkinData.boneNames[boneIndex] = fmt::format("{}", skinnedBones[boneIndex].partialPathName().asChar());
    }
    MPoint mPoint;
    pySkinData.setMaximumVertexWeightCount(boneCount);
    for (UINT vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        UINT influenceIndex = 0;
        UINT weightIndex = vertexIndex * boneCount;
        for (UINT boneIndex = 0; boneIndex < boneCount; boneIndex++)
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
    auto vertexCount = skinData.boneIDs.rows();
    auto vertexWeightsRows = skinData.weights.rows();
    auto influenceCount = skinData.boneIDs.cols();
    auto vertexWeightsCols = skinData.weights.cols();
    if (vertexCount != vertexWeightsRows)
    {
        auto exceptionText = convertStringToChar(
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
        throw std::exception("Error querying bones!");
    }
    auto skinBoneCount = skinnedBones.length();
    auto currentBoneNames = std::vector<std::string>(skinBoneCount);
    for (UINT boneIndex = 0; boneIndex < skinBoneCount; boneIndex++)
    {
        currentBoneNames[boneIndex] = fmt::format("{}", skinnedBones[boneIndex].partialPathName().asChar());
    }
    if (skinBoneCount == 0)
    {
        this->addMissingBones(skinData.boneNames, skinnedBones);
        this->fnSkinCluster.influenceObjects(skinnedBones, &status);
    }
    auto sortedBoneIDs = skinData.getSortedBoneIDs(currentBoneNames);
    if (sortedBoneIDs.unfoundBoneNames.size() != 0)
    {
        skinnedBones.clear();
        this->addMissingBones(sortedBoneIDs.unfoundBoneNames, skinnedBones);
        this->fnSkinCluster.influenceObjects(skinnedBones, &status);
    }

    //validateBindPose(bindPoseMembersArray, skinnedBones);
    auto arraySize = vertexCount * influenceCount;
    MIntArray mBoneIDs(influenceCount);
    MDoubleArray mWeights(arraySize);
    for (UINT influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
    {
        mBoneIDs[influenceIndex] = sortedBoneIDs.sortedBoneIDs[influenceIndex];
    }
    // Unpack nested arrays like so: [[0, 1], [2, 3]] -> [0, 1, 2, 3]
    for (UINT vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        UINT arrayIndex = vertexIndex * influenceCount;
        for (UINT influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
        {
            arrayIndex += influenceIndex;
            auto boneID = skinData.boneIDs(vertexIndex, influenceIndex);
            auto vertexWeight = skinData.weights(vertexIndex, influenceIndex);
            mWeights[arrayIndex] = vertexWeight;
        }
    }
    py::print("mWeights length: ", mWeights.length());
    for (UINT i = 0; i < mWeights.length(); i++)
    {
        py::print(mWeights[i]);
        py::print(mBoneIDs[i]);
        py::print("---");
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
