#pragma once
#include <skin_plus_plus_pymxs.h>



//std::string convertWChartoString(const wchar_t* text)
//{
//	//fmt::format(text);
//	auto text_ = fmt::to_string(text);
//	std::wstring ws(text);
//	return std::string(ws.begin(), ws.end());
//}


INode* getChildByName(const wchar_t* name, INode* parent)
{
    INode* parent_ = parent;
    if (!parent)
    {
        Interface* ip = GetCOREInterface();
        parent_ = ip->GetRootNode();
    }
    INode* node;
    const wchar_t* nodeName;
    for (int i = 0; i < parent_->NumberOfChildren(); i++)
    {
        node = parent_->GetChildNode(i);
        nodeName = node->GetName();
        if (wcscmp(nodeName, name) == 0) return node;
        try { node = getChildByName(name, parent = node); }
        catch (const std::invalid_argument&) { continue; }
        return node;
    }
    throw std::invalid_argument("No node with name: " + convertWCharToChar(name));
}


TriObject* getTriObjectFromNode(INode* node, TimeValue time)
{
    Object* object = node->EvalWorldState(time).obj;
    auto classID = Class_ID(TRIOBJ_CLASS_ID, 0);
    if (object->CanConvertToType(classID))
    {
        TriObject* triObject = (TriObject*)object->ConvertToType(time, classID);
        return triObject;
    }
    else
    {
        return NULL;
    }
}


PolyObject* getPolyObjectFromNode(INode* inNode, TimeValue inTime, bool& deleteIt)
{
    Object* object = inNode->GetObjectRef();
    auto classID = Class_ID(POLYOBJ_CLASS_ID, 0);
    if (object->CanConvertToType(classID))
    {
        PolyObject* polyObject = (PolyObject*)object->ConvertToType(inTime, classID);
        // Note that the polyObject should only be deleted
        // if the pointer to it is not equal to the object

        // pointer that called ConvertToType()
        if (object != polyObject) deleteIt = true;
        return polyObject;
    }
    else
    {
        return NULL;
    }
}


const inline auto getMeshType(INode* node)
{
    ObjectState objectState = node->EvalWorldState(0);

    if (objectState.obj)
    {
        Object* object = objectState.obj->FindBaseObject();
        if (object->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0))) return 0;
        else if (object->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) return 1;
    }
    return -1;
}


bool SkinManager::initialise(const wchar_t* name)
{
    //this->node = getChildByName(name, NULL);
    this->node = GetCOREInterface()->GetINodeByName(name);
    if (!this->node)
    {
        throw py::type_error("SkinData init failed. No node with name: " + convertWCharToChar(name));
    }
    Object* object = this->node->GetObjectRef();
    if (!object || (object->SuperClassID() != GEN_DERIVOB_CLASS_ID))
    {
        throw py::type_error("SkinData init failed. Node is incorrect type " + convertWCharToChar(this->node->GetName()));
    }
    IDerivedObject* iDerivedObject = (IDerivedObject*)(object);
    if (!iDerivedObject)
    {
        this->isValid = false;
        return this->isValid;
    }

    for (int modifierIndex = 0; modifierIndex < iDerivedObject->NumModifiers(); modifierIndex++)
    {
        this->skinModifier = iDerivedObject->GetModifier(modifierIndex);
        if (this->skinModifier->ClassID() != SKIN_CLASSID) continue;

        this->iSkin = (ISkin*)this->skinModifier->GetInterface(I_SKIN);
        if (!this->iSkin) continue;

        this->iSkinContextData = this->iSkin->GetContextInterface(this->node);
        if (!this->iSkinContextData) continue;

        this->iSkinImportData = (ISkinImportData*)this->skinModifier->GetInterface(I_SKINIMPORTDATA);
        if (!this->iSkinImportData) continue;

        this->isValid = true;
        return this->isValid;
    }
    throw std::exception("SkinData init failed on node: " + convertWCharToChar(name));
}


void SkinManager::collectWeightsAndBoneIDs(UINT vertexIndex)
{
    UINT influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
    if (influenceCount > this->maximumVertexWeightCount)
    {
        this->pySkinData->setMaximumVertexWeightCount(influenceCount);
        this->maximumVertexWeightCount = influenceCount;
    }
    for (UINT influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
    {
        double infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
        int influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex);
        this->pySkinData->weights(vertexIndex, influenceIndex) = infuenceWeight;
        this->pySkinData->boneIDs(vertexIndex, influenceIndex) = influenceBoneID;
    }
}


PySkinData* SkinManager::getData()
{
    if (!isValid)
    {
        throw std::exception("SkinData object is invalid. Cannot get skin weights.");
    }

    unsigned int vertexCount = iSkinContextData->GetNumPoints();
    maximumVertexWeightCount = iSkinContextData->GetNumAssignedBones(0);
    pySkinData = new PySkinData(vertexCount, maximumVertexWeightCount);
    auto skinBoneCount = iSkin->GetNumBones();
    pySkinData->boneNames = std::vector<std::string>(skinBoneCount);
    for (auto boneIndex = 0; boneIndex < skinBoneCount; boneIndex++)
    {
        auto boneName = iSkin->GetBoneName(boneIndex);
        if (boneName == NULL)
        {
            auto bone = iSkin->GetBone(boneIndex);
            boneName = bone->GetActualINode()->GetName();
            if (boneName == NULL)
            {
                auto handle = bone->GetHandle();
                auto exceptionText = convertStringToChar(
                    fmt::format("Name is NULL on skinned bone at index: {} with handle: {}", boneIndex + 1, handle)
                );
                throw std::exception(exceptionText);
            }
        }
        pySkinData->boneNames[boneIndex] = convertWCharToString(boneName);
    }
    auto meshType = getMeshType(node);
    if (meshType == 0)
    {
        return getDataMesh(vertexCount);
    }
    else if (meshType == 1)
    {
        return getDataPoly(vertexCount);
    }

    throw std::exception("Unsupported mesh type, convert to EditablePoly or EditableMesh!");
}


PySkinData* SkinManager::getDataPoly(UINT vertexCount)
{

    Matrix3 nodeTransform = node->GetObjectTM(0);
    bool deleteIt;
    PolyObject* polyObject = getPolyObjectFromNode(node, GetCOREInterface()->GetTime(), deleteIt);
    MNMesh& mnMesh = polyObject->GetMesh();
    for (UINT vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        Point3 worldPosition = nodeTransform.PointTransform(mnMesh.V(vertexIndex)->p);
        pySkinData->positions(vertexIndex, 0) = worldPosition.x;
        pySkinData->positions(vertexIndex, 1) = worldPosition.y;
        pySkinData->positions(vertexIndex, 2) = worldPosition.z;
        collectWeightsAndBoneIDs(vertexIndex);
        //py::print("Influence 0 weight: ", pySkinData->weights(vertexIndex, 0));
        //py::print("------");
    };
    return pySkinData;
}


PySkinData* SkinManager::getDataMesh(UINT vertexCount)
{
    Matrix3 nodeTransform = node->GetObjectTM(0);
    TriObject* triObject = getTriObjectFromNode(node, GetCOREInterface()->GetTime());
    Mesh& mesh = triObject->GetMesh();
    for (UINT vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        Point3 worldPosition = nodeTransform.PointTransform(mesh.getVert(vertexIndex));
        pySkinData->positions(vertexIndex, 0) = worldPosition.x;
        pySkinData->positions(vertexIndex, 1) = worldPosition.y;
        pySkinData->positions(vertexIndex, 2) = worldPosition.z;
        collectWeightsAndBoneIDs(vertexIndex);
        //py::print("Influence 0 weight: ", pySkinData->weights(vertexIndex, 0));
        //py::print("------");
    };
    return pySkinData;
}


void SkinManager::addMissingBones(std::vector<std::string> missingBoneNames)
{
    std::vector<INode*> missingBones(missingBoneNames.size());
    for (UINT index = 0; index < missingBoneNames.size(); index++)
	{
		auto missingBoneName = convertStringToWString(missingBoneNames[index]);
		auto missingBone = GetCOREInterface()->GetINodeByName(missingBoneName.c_str());
		if (!missingBone)
		{
			throw py::value_error(
				"No node in scene with name: '" + convertWCharToString(missingBoneName.c_str()) + "'"
			);
		}
		missingBones[index] = missingBone;
	}
	
	for (int index = 0; index < missingBones.size(); index++)
	{
		INode* bone = missingBones[index];
		if (index == missingBones.size())
		{
			this->iSkinImportData->AddBoneEx(bone, true);
		}
		else
		{
			this->iSkinImportData->AddBoneEx(bone, false);
		}
	}
}


struct BoneData
{
	std::vector<std::string> names;
	Tab<INode*> nodes;
	BoneData(int boneCount)
	{
		names = std::vector<std::string>(boneCount);
		nodes = Tab<INode*>();
		nodes.Resize(boneCount);
	};
	~BoneData() {}
};


BoneData getBoneData(ISkin* iSkin, int skinBoneCount)
{
	BoneData boneData(skinBoneCount);
	for (auto boneIndex = 0; boneIndex < skinBoneCount; boneIndex++)
	{
		INode* bone = iSkin->GetBone(boneIndex);
		auto wcharBoneName = bone->GetName();
		auto stringBoneName = convertWCharToString(wcharBoneName);
		boneData.nodes.Append(1, &bone);
		boneData.names[boneIndex] = stringBoneName;
	}
	return boneData;
}


bool SkinManager::setSkinWeights(PySkinData& skinData)
{
	auto boneIDsRows = skinData.boneIDs.rows();
	auto vertexWeightsRows = skinData.weights.rows();
	auto boneIDsCols = skinData.boneIDs.cols();
	auto vertexWeightsCols = skinData.weights.cols();
	if (boneIDsRows != vertexWeightsRows) throw std::length_error(
		"skin bone ids count does not match skin weights count: " + convertWCharToChar(this->node->GetName())
	);
	if (boneIDsCols != vertexWeightsCols) throw std::length_error(
		"skin bone ids count does not match skin weights count: " + convertWCharToChar(this->node->GetName())
	);
	auto vertexCount = this->iSkinContextData->GetNumPoints();
	if (boneIDsRows != vertexCount) throw std::length_error(
		"skin vertex count does not match provided data count: " + convertWCharToChar(this->node->GetName())
	);
	auto skinBoneCount = this->iSkin->GetNumBones();
	if (skinBoneCount == 0)
	{
		this->addMissingBones(skinData.boneNames);
		skinBoneCount = this->iSkin->GetNumBones();
	}

	BoneData boneData = getBoneData(this->iSkin, skinBoneCount);
	SortedBoneNameData sortedBoneIDs = skinData.getSortedBoneIDs(boneData.names);
	Tab<INode*> skinBones = boneData.nodes;
	if (sortedBoneIDs.unfoundBoneNames.size() > 0)
	{
		this->addMissingBones(sortedBoneIDs.unfoundBoneNames);
		skinBoneCount = this->iSkin->GetNumBones();
		boneData = getBoneData(this->iSkin, skinBoneCount);
		sortedBoneIDs = skinData.getSortedBoneIDs(boneData.names);
		skinBones = boneData.nodes;
	}
	for (auto vertexIndex = 0; vertexIndex < boneIDsRows; vertexIndex++)
	{
		Tab<INode*> bones = Tab<INode*>();
        Tab<float> weights = Tab<float>();
		bones.Resize(boneIDsCols);
		weights.Resize(boneIDsCols);
		for (auto influenceIndex = 0; influenceIndex < boneIDsCols; influenceIndex++)
		{
			auto sortedBoneID = sortedBoneIDs.sortedBoneIDs[influenceIndex];
			auto boneID = skinData.boneIDs(vertexIndex, sortedBoneID);
            float influenceWeight = skinData.weights(vertexIndex, sortedBoneID);
			bones.Append(1, &skinBones[boneID]);
			weights.Append(1, &influenceWeight);
		}

		if (!this->iSkinImportData->AddWeights(this->node, vertexIndex, bones, weights))
		{
			return false;
		}
	}

	this->skinModifier->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	return true;
}


bool setSkinWeights(wchar_t* name, PySkinData& skinData)
{
	SkinManager skinData_(name);
	return skinData_.setSkinWeights(skinData);
}


PYBIND11_MODULE(skin_plus_plus_pymxs, m) {
	// This makes the base SkinData class available to the module:
#include <skin_plus_plus_py.h>

	m.def("get_skin_data", [&](wchar_t* name)
		{
			SkinManager skinData(name);
			PySkinData* pySkinData = skinData.getData();
			return pySkinData;
		},
		"Get Skin Data",
		py::arg("name")
	);
	m.def("get_vertex_positions", [&](wchar_t* name)
		{
			SkinManager skinData(name);
			PySkinData* pySkinData = skinData.getData();
			return pySkinData->positions;
		},
		"Get Skin Data",
		py::arg("name")
	);
	m.def("set_skin_weights", [&](wchar_t* name, PySkinData& skinData)
		{
			SkinManager skinManager(name);
			return skinManager.setSkinWeights(skinData);
		},
		"Set Skin Weights",
		py::arg("name"),
		py::arg("skin_data")
	);

	/*py::class_<MAXNode>(m, "MaxNode")
		.def(py::init<INode*>(), py::arg("node"))
		.def("get_name", &MAXNode::SvGetName);*/
}
