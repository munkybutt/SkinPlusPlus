#include <skin_plus_plus_pymaya.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MObject.h>



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
	py::print("Initialised successfully!");

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

static bool isNodeValid(MObject* node)
{
    if (node->hasFn(MFn::kJoint)) return true;
    if (node->hasFn(MFn::kTransform)) return true;
    return false;
}


static MPlug getFreeIndex(MPlug* arrayPlug)
{
    unsigned int freeIndex = arrayPlug->numElements();
    static MPlug elementIndex = arrayPlug->elementByLogicalIndex(freeIndex);
    while (elementIndex.isConnected())
    {
        freeIndex += 1;
        elementIndex = arrayPlug->elementByLogicalIndex(freeIndex);
    }
    return elementIndex;
}


MObject SkinManagerMaya::addMissingBones(BoneNamesVector& missingBoneNames, const UINT& skinnedBoneCount)
{
    MStatus status;
    MSelectionList selectionList;
    MObject dependNode;

    auto bindPosePlug = this->fnSkinCluster.findPlug("bindPose", false);
    MPlugArray connectedPlugs;
    bindPosePlug.connectedTo(connectedPlugs, true, false, &status);
    if (status != MS::kSuccess) throw std::exception("bindPose not connected to array");

    auto bindPoseNode = connectedPlugs[0].node();
    MFnDependencyNode bindPose;
    bindPose.setObject(bindPoseNode);
    if (bindPose.typeName() != "dagPose") throw std::exception("Dependency node not dagPose");

    auto skinClusterMatrixArray = this->fnSkinCluster.findPlug("matrix", false, &status);
    auto skinClusterLockWeightsArray = this->fnSkinCluster.findPlug("lockWeights", false, &status);

    auto bindPoseMembersArray = bindPose.findPlug("members", false);
    auto bindPoseWorldMatrixArray = bindPose.findPlug("worldMatrix", false);
    auto bindPoseParentsArray = bindPose.findPlug("parents", false);
    MDGModifier dagModifier;
    for (UINT i = 0; i < missingBoneNames.size(); i++)
    {
        const auto newBoneIndex = skinnedBoneCount + i;
        selectionList.add(missingBoneNames[i].c_str());
        status = selectionList.getDependNode(i, dependNode);
        if (status != MS::kSuccess) throw std::exception("Failed to get depend node!");
          
        if (!isNodeValid(&dependNode))
        {
            auto exceptionText = convertStringToChar(
                fmt::format("Node is not a joint or transform: {}", missingBoneNames[i])
            );
            throw std::exception(exceptionText);
        }

        MFnIkJoint joint;
        joint.setObject(dependNode);
        auto jointMessage = joint.findPlug("message", false);
        auto jointBindPose = joint.findPlug("bindPose", false);
        auto jointWorldMatrix = joint.findPlug("worldMatrix", false).elementByLogicalIndex(0);
        auto jointLockInfluenceWeightsArray = joint.findPlug("lockInfluenceWeights", false);

        //auto skinClusterMatrixNewElementIndex = skinClusterMatrixArray.elementByLogicalIndex(skinClusterMatrixArray.numElements());
        //auto skinClusterLockWeightsNewElementIndex = skinClusterLockWeightsArray.elementByLogicalIndex(skinClusterLockWeightsArray.numElements());
        //auto bindPoseWorldMatrixNewElementIndex = bindPoseWorldMatrixArray.elementByLogicalIndex(bindPoseWorldMatrixArray.numElements());
        //auto bindPoseMemberNewElementIndex = bindPoseMembersArray.elementByLogicalIndex(bindPoseMembersArray.numElements());
        //auto bindPoseParentNewElementIndex = bindPoseParentsArray.elementByLogicalIndex(bindPoseParentsArray.numElements());

        auto skinClusterMatrixNewElementIndex = getFreeIndex(&skinClusterMatrixArray);
        auto skinClusterLockWeightsNewElementIndex = getFreeIndex(&skinClusterLockWeightsArray);
        auto bindPoseWorldMatrixNewElementIndex = getFreeIndex(&bindPoseWorldMatrixArray);
        auto bindPoseMemberNewElementIndex = getFreeIndex(&bindPoseMembersArray);
        auto bindPoseParentNewElementIndex = getFreeIndex(&bindPoseParentsArray);

        dagModifier.connect(jointMessage, bindPoseMemberNewElementIndex);
        dagModifier.connect(jointBindPose, bindPoseWorldMatrixNewElementIndex);
        dagModifier.connect(jointWorldMatrix, skinClusterMatrixNewElementIndex);
        dagModifier.connect(jointLockInfluenceWeightsArray, skinClusterLockWeightsNewElementIndex);
        dagModifier.connect(bindPoseMemberNewElementIndex, bindPoseParentNewElementIndex);
    }
    status = dagModifier.doIt();
    if (status != MS::kSuccess)
    {
        throw std::exception("Failed to add missing bones!");
    }
    std::string message = "Successfully added missing bones:";
    for (std::string& name : missingBoneNames)
    {
        message += fmt::format(" - ", name);
    }
    py::print(message);
    return bindPoseNode;
}


PySkinData SkinManagerMaya::getData()
{
    if (!this->isValid)
    {
        throw std::exception("SkinData object is invalid. Cannot get skin weights.");
    }
    const UINT vertexCount = this->fnMesh.numVertices();
    if (vertexCount == 0)
    {
        throw std::exception("Mesh has no vertices!");
    }
    MDoubleArray weights;
    UINT boneCount;
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
        UINT weightIndexBase = vertexIndex * boneCount;
        for (UINT boneIndex = 0; boneIndex < boneCount; boneIndex++)
        {
            UINT weightIndex = weightIndexBase + boneIndex;
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
    //pySkinData.weights.hasNaN();

    return pySkinData;
}


void sortBoneIDs(BoneIDsMatrix& boneIDs, std::vector<UINT> newIDOrder, const eg::Index vertexCount, const eg::Index influenceCount)
{
    BoneIDsMatrix sortedBoneIDs = BoneIDsMatrix(boneIDs);
    for (eg::Index vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        for (eg::Index influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
        {
            const int boneID = boneIDs(vertexIndex, influenceIndex);
            if (boneID == -1)
            {
                continue;
            }
            const int newIndex = newIDOrder[boneID];
            boneIDs(vertexIndex, influenceIndex) = newIndex;
        }
    }
}


/// <summary>
/// Convert an eigen::MatrixXd weights matrix to a MDoubleArray.
/// This array is flat, so the weights which are passed in as a vertex count x influence count matrix, need to be
/// structured so they are flat but each number of elements in the array still represents vertex count x influence count.
/// 
/// For example:
/// matrix[0][3] == MDoubleArray[3]
/// matrix[3][3] == MDoubleArray[9]
/// 
/// </summary>
/// <param name="boneIDs"></param>
/// <param name="weights"></param>
/// <param name="vertexCount"></param>
/// <param name="influenceCount"></param>
/// <returns></returns>
MDoubleArray getWeightsAsMDoubleArray(BoneIDsMatrix& boneIDs, WeightsMatrix& weights, const eg::Index& vertexCount, const eg::Index& influenceCount, const eg::Index& maxInfluenceCount)
{
    const UINT arraySize = vertexCount * maxInfluenceCount;
    MDoubleArray mWeights(arraySize, 0.0);
    for (eg::Index vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        const UINT baseIndex = vertexIndex * maxInfluenceCount;
        for (eg::Index influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
        {
            const int boneID = boneIDs(vertexIndex, influenceIndex);
            if (boneID == -1)
            {
                continue;
            }
            const double weight = weights(vertexIndex, influenceIndex);
            const UINT mWeightsIndex = baseIndex + boneID;
            mWeights[mWeightsIndex] = weight;
        }
    }
    //for (eg::Index i = 0; i < vertexCount; i++)
    //{
    //    auto message = fmt::format("Vertex: {} - ", i);
    //    for (UINT j = 0; j < influenceCount; j++)
    //    {
    //        if (j == 0)
    //        {
    //            message += "[";
    //        }
    //        auto mIndex = i * influenceCount + j;
    //        message += fmt::format("{}: {}", mIndex, mWeights[mIndex]);
    //        if (j < influenceCount - 1)
    //        {
    //            message += ", ";
    //        }
    //    }
    //    message += "]";
    //    py::print(message);
    //}
    //for (size_t i = 0; i < mWeights.length(); i++)
    //{
    //    py::print(mWeights[i]);
    //}
    return mWeights;
}


void getBoneNames(std::vector<std::string>& currentBoneNames, const MDagPathArray& skinnedBones, const UINT& skinnedBoneCount)
{
    currentBoneNames.clear();
    currentBoneNames.resize(skinnedBoneCount);
    for (UINT boneIndex = 0; boneIndex < skinnedBoneCount; boneIndex++)
    {
        currentBoneNames[boneIndex] = fmt::format("{}", skinnedBones[boneIndex].partialPathName().asChar());
    }
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
    auto skinnedBoneCount = skinnedBones.length();
    auto currentBoneNames = std::vector<std::string>(skinnedBoneCount);
    getBoneNames(currentBoneNames, skinnedBones, skinnedBoneCount);
    bool bonesAdded = false;
    if (skinnedBoneCount == 0)
    {
        this->addMissingBones(skinData.boneNames, skinnedBoneCount);
        bonesAdded = true;
    }
    auto sortedBoneIDs = skinData.getSortedBoneIDs(currentBoneNames);
    if (sortedBoneIDs.unfoundBoneNames.size() != 0)
    {
        this->addMissingBones(sortedBoneIDs.unfoundBoneNames, skinnedBoneCount);
        bonesAdded = true;
    }
    if (bonesAdded)
    {
        skinnedBones.clear();
        this->fnSkinCluster.influenceObjects(skinnedBones, &status);
        skinnedBoneCount = skinnedBones.length();
        getBoneNames(currentBoneNames, skinnedBones, skinnedBoneCount);
        if (status != MS::kSuccess)
        {
            throw std::exception("Error querying bones!");
        }
        sortedBoneIDs = skinData.getSortedBoneIDs(currentBoneNames);
        if (sortedBoneIDs.unfoundBoneNames.size() != 0)
        {
            throw std::exception("Could not find all bones required!");
        }
    }
    const auto sortedBoneSize = sortedBoneIDs.sortedBoneIDs.size();
    MIntArray mBoneIDs(sortedBoneSize);
    for (size_t index = 0; index < sortedBoneSize; index++)
    {
        mBoneIDs[index] = sortedBoneIDs.sortedBoneIDs[index];
    }

    MDoubleArray mWeights = getWeightsAsMDoubleArray(skinData.boneIDs, skinData.weights, vertexCount, influenceCount, sortedBoneSize);
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
