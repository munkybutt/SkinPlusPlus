#pragma once
#include <skin_plus_plus_pymxs.h>


char convertWCharToChar(const wchar_t* text)
{
	size_t length = std::wcslen(text);
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> conv;
	std::string storeTextBuffer = conv.to_bytes(text, text + length);

	return storeTextBuffer[0];
}

std::string convertWCharToString(const wchar_t* text)
{
	size_t length = std::wcslen(text);
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> conv;
	std::string storeTextBuffer = conv.to_bytes(text, text + length);

	return storeTextBuffer;
}

std::wstring convertStringToWString(const std::string& multi) {
	std::wstring wide;
	wchar_t w;
	mbstate_t mb{};
	size_t n = 0, len = multi.length() + 1;
	while (auto res = mbrtowc(&w, multi.c_str() + n, len - n, &mb)) {
		if (res == size_t(-1) || res == size_t(-2))
			throw "invalid encoding";

		n += res;
		wide += w;
	}
	return wide;
}


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
		try { node = getChildByName(name, parent = node); } catch (const std::invalid_argument&) { continue; }
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
	this->node = getChildByName(name, NULL);
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


void SkinManager::collectWeightsAndBoneIDs(unsigned int vertexIndex)
{
	auto influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
	if (influenceCount > this->maximumVertexWeightCount)
	{
		this->pySkinData->setMaximumVertexWeightCount(influenceCount);
		this->maximumVertexWeightCount = influenceCount;
	}
	for (auto influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
	{
		auto infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
		if (infuenceWeight <= 0.0f) continue;
		auto influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex);
		this->pySkinData->weights(vertexIndex, influenceIndex) = infuenceWeight;
		this->pySkinData->boneIDs(vertexIndex, influenceIndex) = influenceBoneID;
	}
}


PySkinData* SkinManager::getData()
{
	if (!this->isValid)
	{
		throw std::exception("SkinData object is invalid. Cannot get skin weights.");
	}

	unsigned int vertexCount = this->iSkinContextData->GetNumPoints();
	this->maximumVertexWeightCount = this->iSkinContextData->GetNumAssignedBones(0);
	this->pySkinData = new PySkinData(vertexCount, this->maximumVertexWeightCount);
	auto skinBonesCount = this->iSkin->GetNumBones();
	this->pySkinData->boneNames = std::vector<std::string>(skinBonesCount);
	for (auto boneIndex = 0; boneIndex < skinBonesCount; boneIndex++)
	{
		this->pySkinData->boneNames[boneIndex] = convertWCharToString(this->iSkin->GetBone(boneIndex)->GetName());
	}
	auto meshType = getMeshType(this->node);
	if (meshType == 0)
	{
		return this->getDataMesh(vertexCount);
	}
	else if (meshType == 1)
	{
		return this->getDataPoly(vertexCount);
	}

	throw std::exception("Unsupported mesh type, convert to EditablePoly or EditableMesh!");
}


PySkinData* SkinManager::getDataPoly(int vertexCount)
{

	Matrix3 nodeTransform = this->node->GetObjectTM(0);
	bool deleteIt;
	PolyObject* polyObject = getPolyObjectFromNode(this->node, GetCOREInterface()->GetTime(), deleteIt);
	MNMesh& mnMesh = polyObject->GetMesh();
	for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		Point3 worldPosition = nodeTransform.PointTransform(mnMesh.V(vertexIndex)->p);
		this->pySkinData->positions(vertexIndex, 0) = worldPosition.x;
		this->pySkinData->positions(vertexIndex, 1) = worldPosition.y;
		this->pySkinData->positions(vertexIndex, 2) = worldPosition.z;
		this->collectWeightsAndBoneIDs(vertexIndex);
	};
	return this->pySkinData;
}


PySkinData* SkinManager::getDataMesh(int vertexCount)
{
	Matrix3 nodeTransform = this->node->GetObjectTM(0);
	TriObject* triObject = getTriObjectFromNode(node, GetCOREInterface()->GetTime());
	Mesh& mesh = triObject->GetMesh();
	for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		Point3 worldPosition = nodeTransform.PointTransform(mesh.getVert(vertexIndex));
		this->pySkinData->positions(vertexIndex, 0) = worldPosition.x;
		this->pySkinData->positions(vertexIndex, 1) = worldPosition.y;
		this->pySkinData->positions(vertexIndex, 2) = worldPosition.z;
		this->collectWeightsAndBoneIDs(vertexIndex);
	};
	return this->pySkinData;
}


//void addBonesOpperator(ISkinImportData* inSkinImportData, ISkin* inSkinModifier, Array* inBones)
//{
//	MaxSDK::Array<ULONG> currentSkinModifierHandles = getSkinModifierBoneHandles(inSkinModifier);
//	for (int boneIndex = 0; boneIndex < inBones->size; boneIndex++)
//	{
//		Value* mxsBone = inBones->data[boneIndex];
//		ULONG nodeHandle = mxsBone->to_node()->GetHandle();
//		if (currentSkinModifierHandles.find(nodeHandle) == -1)
//		{
//			INode* bone = mxsBone->to_node();
//			if (boneIndex == inBones->size)
//			{
//				inSkinImportData->AddBoneEx(bone, true);
//			}
//			else
//			{
//				inSkinImportData->AddBoneEx(bone, false);
//			}
//			currentSkinModifierHandles.append(nodeHandle);
//		}
//	}
//}
//
//static bool addBones(Value* inSkinnedNode, Array* ioBones)
//{
//	INode* node = inSkinnedNode->to_node();
//	SkinDescription* skinDescription = getSkinDescription(node);
//	if (!skinDescription)
//	{
//		return false;
//	}
//
//	//MaxSDK::Array<ULONG> currentSkinModifierHandles = getSkinModifierBoneHandles(skinDescription);
//	//for (int boneIndex = 0; boneIndex < ioBones->size; boneIndex++)
//	//{
//	//	Value* mxsBone = ioBones->data[boneIndex];
//	//	if (!is_node(mxsBone))
//	//	{
//	//		throw RuntimeError(_T("Can only add nodes to skin modifier! "), mxsBone);
//	//	}
//
//	//	ULONG nodeHandle = mxsBone->to_node()->GetHandle();
//	//	if (currentSkinModifierHandles.find(nodeHandle) == -1)
//	//	{
//	//		INode* bone = mxsBone->to_node();
//	//		if (boneIndex == ioBones->size)
//	//		{
//	//			skinDescription->skinImportData->AddBoneEx(bone, true);
//	//		}
//	//		else
//	//		{
//	//			skinDescription->skinImportData->AddBoneEx(bone, false);
//	//		}
//	//		currentSkinModifierHandles.append(nodeHandle);
//	//	}
//	//}
//	addBonesOpperator(skinDescription->skinImportData, skinDescription->skin, ioBones);
//	skinDescription->boneModData->reevaluate = TRUE;
//	skinDescription->modifier->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
//	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
//
//	return true;
//}
//
//static bool addBones(Modifier* inSkinModifier, Array* inBones)
//{
//	ISkinImportData* skinImportData = (ISkinImportData*)inSkinModifier->GetInterface(I_SKINIMPORTDATA);
//	if (!skinImportData)
//	{
//		return false;
//	}
//	ISkin* skin = (ISkin*)inSkinModifier->GetInterface(I_SKIN);
//	if (!skin)
//	{
//		return false;
//	}
//	addBonesOpperator(skinImportData, skin, inBones);
//	inSkinModifier->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
//	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
//
//	return true;
//}
//
//
//static Value* addSkinModifier(Array* inNodes, Value* inBones = NULL, int inInflucentCount = 8/*, bool inAlwaysDeform=true*/)
//{
//	Array* bones = NULL;
//	if (inBones && inBones != &undefined)
//	{
//		bones = (Array*)inBones;
//	}
//	Array* outArray = new Array(inNodes->size);
//	for (int nodeIndex = 0; nodeIndex < inNodes->size; nodeIndex++)
//	{
//		MAXNode* mxsNode = (MAXNode*)inNodes->data[nodeIndex];
//		INode* node = mxsNode->to_node();
//		Modifier* skinModifier = getSkinModifier(node);
//		if (!skinModifier)
//		{
//			skinModifier = (Modifier*)GetCOREInterface()->CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);
//			GetCOREInterface12()->AddModifier(*node, *skinModifier);
//		}
//		if (bones)
//		{
//			addBones(skinModifier, bones);
//		}
//		IParamBlock2* skinParamBlock = skinModifier->GetParamBlock(3);
//		skinParamBlock->SetValue(skin_advance_bonelimit, GetCOREInterface()->GetTime(), inInflucentCount);
//		outArray->append(MAXModifier::intern(skinModifier));
//	}
//	return outArray;
//}


//Get the Handles of all bones in the given skin modifier.
static std::vector<ULONG> getSkinnedBoneHandles(ISkin* skinModifier)
{
	const int boneCount = skinModifier->GetNumBones();
	std::vector<ULONG> skinnedBoneIDs(boneCount);
	for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
	{
		INode* bone = skinModifier->GetBone(boneIndex);
		skinnedBoneIDs[boneIndex] = bone->GetHandle();
	}

	return skinnedBoneIDs;
}


void SkinManager::addMissingBones(std::vector<std::string> missingBoneNames)
{
	std::vector<INode*> missingBones(missingBoneNames.size());
	for (size_t index = 0; index < missingBoneNames.size(); index++)
	{
		py::print(missingBoneNames[index]);
		auto missingBoneName = convertStringToWString(missingBoneNames[index]);
		auto missingBone = getChildByName(missingBoneName.c_str(), NULL);
		if (!missingBone)
		{
			throw py::value_error("No node in scene with name: '" + convertWCharToString(missingBoneName.c_str()) + "'");
		}
		missingBones[index] = missingBone;
	}
	
	std::vector<ULONG> skinnedBoneHandles = getSkinnedBoneHandles(this->iSkin);
	for (int index = 0; index < missingBones.size(); index++)
	{
		INode* bone = missingBones[index];
		ULONG nodeHandle = bone->GetHandle();
		ULONG nodeHandleIndex = getItemIndex(skinnedBoneHandles, nodeHandle);
		if (nodeHandleIndex == NULL)
		{
			if (index == missingBones.size())
			{
				this->iSkinImportData->AddBoneEx(bone, true);
			}
			else
			{
				this->iSkinImportData->AddBoneEx(bone, false);
			}
			skinnedBoneHandles.push_back(nodeHandle);
		}
	}
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
	Tab<INode*> skinBones;
	skinBones.Resize(boneIDsCols);
	auto skinBonesCount = this->iSkin->GetNumBones();
	std::vector<std::string> skinBoneNames(skinBonesCount);
	for (auto boneIndex = 0; boneIndex < skinBonesCount; boneIndex++)
	{
		INode* bone = this->iSkin->GetBone(boneIndex);
		auto wcharBoneName = bone->GetName();
		auto stringBoneName = convertWCharToString(wcharBoneName);
		skinBones.Append(1, &bone);
		skinBoneNames[boneIndex] = stringBoneName;
	}
	SortedBoneNameData sortedBoneIDs = skinData.getSortedBoneIDs(skinBoneNames);
	if (sortedBoneIDs.unfoundBoneNames.size() > 0)
	{
		this->addMissingBones(sortedBoneIDs.unfoundBoneNames);
	}
	sortedBoneIDs = skinData.getSortedBoneIDs(skinBoneNames);
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
			auto influenceWeight = skinData.weights(vertexIndex, sortedBoneID);
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

	//m.def("__repr__", [](const PySkinData& pySkinData) {
	//	return "PySkinData<size:" + pySkinData.boneIDs.size() + "asdf>";
	//	}
	//);
}