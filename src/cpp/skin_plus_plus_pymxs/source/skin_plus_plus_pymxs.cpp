#pragma once
#include <skin_plus_plus_pymxs.h>


#define GET_POLY_DATA(node)                                                                    \
Matrix3 nodeTransform = node->GetObjectTM(0);                                                  \
bool deleteIt;                                                                                 \
PolyObject* polyObject = getPolyObjectFromNode(node, GetCOREInterface()->GetTime(), deleteIt); \
MNMesh& mnMesh = polyObject->GetMesh();

#define GET_MESH_DATA(node)                                                       \
Matrix3 nodeTransform = node->GetObjectTM(0);                                     \
TriObject* triObject = getTriObjectFromNode(node, GetCOREInterface()->GetTime()); \
Mesh& mesh = triObject->GetMesh();


static inline TriObject* getTriObjectFromNode(INode* node, TimeValue time)
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


static inline PolyObject* getPolyObjectFromNode(INode* inNode, TimeValue inTime, bool& deleteIt)
{
    Object* object = inNode->GetObjectRef();
    auto classID = Class_ID(POLYOBJ_CLASS_ID, 0);
    if (object->CanConvertToType(classID))
    {
        //auto po = static_cast<PolyObject*>(object->ConvertToType(inTime, classID));
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


enum MeshType
{
    mesh,
    poly,
    none
};


static inline MeshType getMeshType(INode* node)
{
    ObjectState objectState = node->EvalWorldState(0);

    if (objectState.obj)
    {
        Object* object = objectState.obj->FindBaseObject();
        if (object->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0))) return MeshType::mesh;
        if (object->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) return MeshType::poly;
    }
    return MeshType::none;
}


static const inline int getVertexCount(INode* node)
{
    const MeshType meshType = getMeshType(node);
    if (meshType == MeshType::mesh)
    {
        GET_MESH_DATA(node);
            return mesh.getNumVerts();
    }
    else if (meshType == MeshType::poly)
    {
        GET_POLY_DATA(node)
            return mnMesh.VNum();
    }
    throw py::type_error("Incorrect mesh type!");
}


bool SkinManager::initialiseSkin()
{
    if (!this->node)
    {
        throw py::type_error("SkinData init failed. No node initialised!");
    }
    py::print(fmt::format(L"Node name: {}", node->GetName()));
    Object* object = this->node->GetObjectRef();
    if (!object || (object->SuperClassID() != GEN_DERIVOB_CLASS_ID))
    {
        /*fmt::format(L"SkinData init failed. Node is incorrect type: ", this->node->GetName())*/
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
    return this->isValid;
}


bool SkinManager::initialise(const wchar_t* name)
{
    this->node = GetCOREInterface()->GetINodeByName(name);
    if (!this->node)
    {
        throw py::type_error("SkinData init failed. No node with name: " + convertWCharToChar(name));
    }
    if (this->initialiseSkin())
    {
        return true;
    }
    throw std::runtime_error("SkinData init failed on node: " + convertWCharToChar(name));
}


bool SkinManager::initialise(ULONG handle)
{
    this->node = GetCOREInterface()->GetINodeByHandle(handle);
    if (!this->node)
    {
        throw py::type_error(fmt::format("SkinData init failed. No node with handle: {}", handle));
    }
    if (this->initialiseSkin())
    {
        return true;
    }
    throw std::runtime_error(fmt::format("SkinData init failed on node with handle: {}", handle));
}


PySkinData* SkinManager::extractSkinData(std::optional<VertexIDsMatrix> vertexIDs)
{
    if (!isValid)
    {
        throw std::runtime_error("SkinData object is invalid. Cannot get skin weights.");
    }

    this->maximumVertexWeightCount = iSkinContextData->GetNumAssignedBones(0);
    UINT vertexCount = 0;
    if (vertexIDs.has_value())
    {
        const VertexIDsMatrix vertexIDs_ = vertexIDs.value();
        vertexCount = vertexIDs_.cols();
        for (UINT arrayIndex = 0; arrayIndex < vertexIDs_.size(); arrayIndex++)
        {
            const int vertexIndex = vertexIDs_[arrayIndex];
            const int influenceCount = iSkinContextData->GetNumAssignedBones(vertexIndex);
            if (influenceCount > this->maximumVertexWeightCount)
            {
                this->maximumVertexWeightCount = influenceCount;
            }
        }
    }
    else
    {
        vertexCount = iSkinContextData->GetNumPoints();
        for (UINT i = 0; i < vertexCount; i++)
        {
            const int influenceCount = iSkinContextData->GetNumAssignedBones(i);
            if (influenceCount > this->maximumVertexWeightCount)
            {
                this->maximumVertexWeightCount = influenceCount;
            }
        }
    }
    if (vertexIDs.has_value())
    {
        this->pySkinData = new PySkinData(vertexIDs.value(), this->maximumVertexWeightCount);
    }
    else
    {
        this->pySkinData = new PySkinData(vertexCount, this->maximumVertexWeightCount);
    }
    const int skinBoneCount = iSkin->GetNumBones();
    this->pySkinData->boneNames = std::vector<std::string>(skinBoneCount);
    for (auto boneIndex = 0; boneIndex < skinBoneCount; boneIndex++)
    {
        // we don't use GetBoneName as that can return nulls depending on how the skin modifier has been setup
        auto bone = iSkin->GetBone(boneIndex);
        if (!bone)
        {
            bone->GetActualINode();
        }
        auto boneName = bone->GetName();
        if (!boneName)
        {
            auto handle = bone->GetHandle();
            throw std::runtime_error(
                fmt::format("Name is NULL on skinned bone at index: {} with handle: {}", boneIndex + 1, handle)
            );
        }
        this->pySkinData->boneNames[boneIndex] = convertWCharToString(boneName);
    }
    auto meshType = getMeshType(node);
    if (meshType == MeshType::mesh)
    {
        if (vertexIDs.has_value())
        {
            this->extractDataMesh(vertexCount, vertexIDs.value());
        }
        else
        {
            this->extractDataMesh(vertexCount);
        }
        return this->pySkinData;
    }
    else if (meshType == MeshType::poly)
    {
        if (vertexIDs.has_value())
        {
            this->extractDataPoly(vertexCount, vertexIDs.value());
        }
        else
        {
            this->extractDataPoly(vertexCount);
        }
        return this->pySkinData;
    }

    throw std::runtime_error("Unsupported mesh type, convert to EditablePoly or EditableMesh!");
}


inline void SkinManager::extractWeightsAndBoneIDs(const UINT vertexIndex, const UINT arrayIndex)
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
        this->pySkinData->weights(arrayIndex, influenceIndex) = infuenceWeight;
        this->pySkinData->boneIDs(arrayIndex, influenceIndex) = influenceBoneID;
    }
}


inline void SkinManager::extractWeightsAndBoneIDs(const UINT vertexIndex)
{
    extractWeightsAndBoneIDs(vertexIndex, vertexIndex);
}


inline void SkinManager::extractDataPoly(const UINT vertexCount, VertexIDsMatrix vertexIDs)
{
    GET_POLY_DATA(this->node);
    for (UINT arrayIndex = 0; arrayIndex < vertexCount; arrayIndex++)
    {
        UINT vertexID = vertexIDs(arrayIndex, 0);
        Point3 worldPosition = nodeTransform.PointTransform(mnMesh.V(vertexID)->p);
        this->pySkinData->positions(arrayIndex, 0) = worldPosition.x;
        this->pySkinData->positions(arrayIndex, 1) = worldPosition.y;
        this->pySkinData->positions(arrayIndex, 2) = worldPosition.z;
        extractWeightsAndBoneIDs(vertexID, arrayIndex);
    };
}


inline void SkinManager::extractDataPoly(const UINT vertexCount)
{
    GET_POLY_DATA(this->node);
    for (UINT vertexID = 0; vertexID < vertexCount; vertexID++)
    {
        Point3 worldPosition = nodeTransform.PointTransform(mnMesh.V(vertexID)->p);
        this->pySkinData->positions(vertexID, 0) = worldPosition.x;
        this->pySkinData->positions(vertexID, 1) = worldPosition.y;
        this->pySkinData->positions(vertexID, 2) = worldPosition.z;
        extractWeightsAndBoneIDs(vertexID);
    };
}


inline void SkinManager::extractDataMesh(const UINT vertexCount, VertexIDsMatrix vertexIDs)
{
    GET_MESH_DATA(this->node)
    for (UINT vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        UINT vertexID = vertexIDs(vertexIndex, 0);
        Point3 worldPosition = nodeTransform.PointTransform(mesh.getVert(vertexID));
        this->pySkinData->positions(vertexID, 0) = worldPosition.x;
        this->pySkinData->positions(vertexID, 1) = worldPosition.y;
        this->pySkinData->positions(vertexID, 2) = worldPosition.z;
        extractWeightsAndBoneIDs(vertexID, vertexIndex);
    };
}


inline void SkinManager::extractDataMesh(const UINT vertexCount)
{
    GET_MESH_DATA(this->node)
    for (UINT vertexID = 0; vertexID < vertexCount; vertexID++)
    {
        Point3 worldPosition = nodeTransform.PointTransform(mesh.getVert(vertexID));
        this->pySkinData->positions(vertexID, 0) = worldPosition.x;
        this->pySkinData->positions(vertexID, 1) = worldPosition.y;
        this->pySkinData->positions(vertexID, 2) = worldPosition.z;
        extractWeightsAndBoneIDs(vertexID);
    };
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
	BoneData(const int boneCount)
	{
		names = std::vector<std::string>(boneCount);
		nodes = Tab<INode*>();
		nodes.Resize(boneCount);
	};
	~BoneData() {}
};


inline static BoneData getBoneData(ISkin* iSkin, const int skinBoneCount)
{
    BoneData boneData = BoneData(skinBoneCount);
	for (auto boneIndex = 0; boneIndex < skinBoneCount; boneIndex++)
	{
		INode* bone = iSkin->GetBone(boneIndex);
		auto wcharBoneName = bone->GetName();
		std::string stringBoneName = convertWCharToString(wcharBoneName);
        boneData.nodes.Append(1, &bone);
		boneData.names[boneIndex] = stringBoneName;
	}
	return boneData;
}


bool SkinManager::applySkinData(PySkinData& skinData)
{
    const eg::Index boneIDsRows = skinData.boneIDs.rows();
    const eg::Index vertexWeightsRows = skinData.weights.rows();
    const eg::Index boneIDsCols = skinData.boneIDs.cols();
    const eg::Index vertexWeightsCols = skinData.weights.cols();
	if (boneIDsRows != vertexWeightsRows) throw std::length_error(
		"skin bone ids count does not match skin weights count: " + convertWCharToChar(this->node->GetName())
	);
	if (boneIDsCols != vertexWeightsCols) throw std::length_error(
		"skin bone ids count does not match skin weights count: " + convertWCharToChar(this->node->GetName())
	);
	const int vertexCount = getVertexCount(node);
	if (boneIDsRows != vertexCount && !skinData.vertexIDs.has_value()) throw std::length_error(
        fmt::format("skin vertex count does not match provided data count: {}", convertWCharToChar(this->node->GetName()))
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
    const size_t sortedBoneIDCount = sortedBoneIDs.sortedBoneIDs.size();
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
			const int boneID = skinData.boneIDs(vertexIndex, influenceIndex);
            if (boneID == -1)
            {
                // If boneID is -1, then there are no more influence weights for this vertex so break out of the loop.
                break;
            }
            if (boneID > skinBoneCount)
            {
                continue;
            }
            if (boneID > sortedBoneIDCount)
            {
                const auto exceptionText = fmt::format(
                    "Influence ID: {} is out of range of sorted bone count {}!",
                    boneID,
                    sortedBoneIDCount
                );
                throw std::length_error(exceptionText);
            }
            float influenceWeight = skinData.weights(vertexIndex, influenceIndex);
			const UINT sortedBoneID = sortedBoneIDs.sortedBoneIDs[boneID];
			bones.Append(1, &skinBones[sortedBoneID]);
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


PYBIND11_MODULE(skin_plus_plus_pymxs, m) {
	// This makes the base SkinData class available to the module:
#include <skin_plus_plus_py.h>

    m.def("extract_skin_data", [&](wchar_t* name, std::optional<VertexIDsMatrix> vertexIDs = std::nullopt)
        {
            SkinManager skinData(name);
            PySkinData* pySkinData = skinData.extractSkinData(vertexIDs);
            return pySkinData;
        },
        "Extract SkinData from the mesh with the given name",
        py::arg("name"),
        py::arg("vertex_ids") = py::none()
    );
    m.def("extract_skin_data", [&](ULONG handle, std::optional<VertexIDsMatrix> vertexIDs = std::nullopt)
        {
            SkinManager skinData(handle);
            PySkinData* pySkinData = skinData.extractSkinData(vertexIDs);
            return pySkinData;
        },
        "Extract SkinData from the mesh with the given handle",
        py::arg("handle"),
        py::arg("vertex_ids") = py::none()
    );
    m.def("apply_skin_data", [&](wchar_t* name, PySkinData& skinData)
        {
            SkinManager skinManager(name);
            return skinManager.applySkinData(skinData);
        },
        "Apply SkinData to the mesh with the given name",
		py::arg("name"),
		py::arg("skin_data")
	);
    m.def("apply_skin_data", [&](ULONG handle, PySkinData& skinData)
        {
            SkinManager skinManager(handle);
            return skinManager.applySkinData(skinData);
        },
        "Apply SkinData to the mesh with the given name",
        py::arg("name"),
        py::arg("skin_data")
    );
    m.def("get_vertex_positions", [&](wchar_t* name)
        {
            SkinManager skinData(name);
            PySkinData* pySkinData = skinData.extractSkinData();
            return pySkinData->positions;
        },
        "Get Skin Data",
        py::arg("name")
    );
}
