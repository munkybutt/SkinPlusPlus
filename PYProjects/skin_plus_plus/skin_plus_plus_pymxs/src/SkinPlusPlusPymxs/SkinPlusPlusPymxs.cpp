#pragma once
#include <SkinPlusPlusPymxs.h>


char convertWCharToChar(const wchar_t* text)
{
	size_t length = std::wcslen(text);
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> conv;
	std::string StoreTextBuffer = conv.to_bytes(text, text + length);

	return StoreTextBuffer[0];
}


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
	//name->length();
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


bool applyOffsetToVertices(INode* node, Point3 offset)
{
	static bool success = false;
	//bool deleteIt = false;
	TriObject* triObject = getTriObjectFromNode(node, GetCOREInterface()->GetTime());
	if (triObject)
	{
		Mesh& mesh = triObject->GetMesh();
		for (int vertIndex = 0; vertIndex < mesh.getNumVerts(); vertIndex++)
		{
			Point3 position = mesh.getVert(vertIndex);
			mesh.setVert(vertIndex, position + offset);
		}
		mesh.InvalidateGeomCache();
		mesh.InvalidateTopologyCache();
		node->SetObjectRef(triObject);
		triObject->NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);
		Interface* coreInterface = GetCOREInterface();
		coreInterface->RedrawViews(coreInterface->GetTime());
		success = true;
	}
	return success;
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


//static bool applyOffsetToVertices(INode* inNode, Point3 inOffset)
//{
//	bool deleteIt = false;
//	Interface* coreInterface = GetCOREInterface();
//	PolyObject* polyObject = getPolyObjectFromNode(inNode, coreInterface->GetTime(), deleteIt);
//	if (polyObject)
//	{
//		MNMesh& mnMesh = polyObject->GetMesh();
//		for (int vertIndex = 0; vertIndex < mnMesh.VNum(); vertIndex++)
//		{
//			Point3 position = mnMesh.V(vertIndex)->p;
//		}
//		if (deleteIt) polyObject->DeleteMe();
//		return true;
//	}
//	return false;
//}


bool SkinData::initialise(const wchar_t* name)
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
	this->iDerivedObject = (IDerivedObject*)(object);
	if (!this->iDerivedObject)
	{
		this->isValid = false;
		return this->isValid;
	}

	for (int modifierIndex = 0; modifierIndex < this->iDerivedObject->NumModifiers(); modifierIndex++)
	{
		this->skinModifier = this->iDerivedObject->GetModifier(modifierIndex);
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


std::vector<std::vector<std::vector <float>>> SkinData::getSkinWeights()
{
	if (!this->isValid)
	{
		throw std::exception("SkinData object is invalid. Cannot get skin weights.");
	}
	unsigned int vertexCount = this->iSkinContextData->GetNumPoints();
	std::vector<std::vector<std::vector <float>>> skinDataArray(
		2, std::vector<std::vector<float>>(vertexCount)
	);
	for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		auto influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
		std::vector<float> influenceWeights(influenceCount);
		std::vector<float> influenceBoneIDs(influenceCount);
		skinDataArray[0][vertexIndex] = influenceWeights;  //influenceWeights
		skinDataArray[1][vertexIndex] = influenceBoneIDs;  //influenceBoneIDs
		for (auto influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
		{
			auto infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
			if (infuenceWeight <= 0.0f) continue;
			auto influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex);
			influenceWeights[influenceIndex] = infuenceWeight;
			influenceBoneIDs[influenceIndex] = float(influenceBoneID);
		}
	};
	return skinDataArray;
}


void PySkinData::setInternalState(py::tuple data)
{
	this->boneIDs = py::cast<eg::MatrixXi>(data[0]);
	this->weights = py::cast<eg::MatrixXf>(data[1]);
	this->positions = py::cast<eg::MatrixXf>(data[2]);
}


const inline auto getMeshType(INode* node)
{
	ObjectState objectState = node->EvalWorldState(0);

	if (objectState.obj)
	{
		Object* object = objectState.obj->FindBaseObject();
		//if (object->ClassID() == Class_ID(EDITTRIOBJ_CLASS_ID, 0)) return 0;
		//else if (object->ClassID() == EPOLYOBJ_CLASS_ID) return 1;
		if (object->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0))) return 0;
		else if (object->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) return 1;
	}
	return -1;
}

//PySkinData* SkinData::getData()
//{
//	if (!this->isValid)
//	{
//		throw std::exception("SkinData object is invalid. Cannot get skin weights.");
//	}
//	unsigned int vertexCount = this->iSkinContextData->GetNumPoints();
//	py::print(vertexCount);
//	PySkinData* pySkinData = new PySkinData(vertexCount, 8);
//	py::print(pySkinData);
//
//	ObjectState objectState = this->node->EvalWorldState(0);
//	py::print("objectState");
//
//	//if (objectState.obj)
//	//{
//	//	//Look at the super class ID to determine the type of the object.
//
//	//	if (objectState.obj->IsSubClassOf(triObjectClassID))
//	//	{
//
//	//		TriObject* triObject = (TriObject*)objectState.obj;	// Get a mesh from input object
//	//		Mesh* mesh = &triObject->GetMesh();
//	//		int numVert = mesh->getNumVerts();
//	//		int numFaces = mesh->getNumFaces();
//	Object* baseObject = objectState.obj->FindBaseObject();
//	py::print("baseObject");
//	//PolyObject* polyObject = (PolyObject*)baseObject;
//	EPoly* iePoly = (EPoly*)(baseObject->GetInterface(EPOLY_INTERFACE));
//	py::print("iePoly");
//
//	//This is where you get the Mesh and loop for all the verts.
//	MNMesh* mnMesh = iePoly->GetMeshPtr();
//	py::print("mnMesh");
//	for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
//	{
//		py::print(vertexIndex);
//		Point3 position = mnMesh->v[vertexIndex].p;
//		pySkinData->positions(vertexIndex, 0) = position.x;
//		pySkinData->positions(vertexIndex, 1) = position.y;
//		pySkinData->positions(vertexIndex, 2) = position.z;
//		auto influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
//		for (auto influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
//		{
//			auto infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
//			if (infuenceWeight <= 0.0f) continue;
//			auto influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex);
//			pySkinData->weights(vertexIndex, influenceIndex) = infuenceWeight;
//			pySkinData->boneIDs(vertexIndex, influenceIndex) = influenceBoneID;
//		}
//	};
//	return pySkinData;
//}

void SkinData::collectWeightsAndBoneIDs(PySkinData* pySkinData, unsigned int vertexIndex)
{
	auto influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
	if (influenceCount > this->maxInfluenceCount)
	{
		pySkinData->setMaximumInfluenceCount(influenceCount);
		this->maxInfluenceCount = influenceCount;
	}
	for (auto influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
	{
		auto infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
		if (infuenceWeight <= 0.0f) continue;
		auto influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex);
		pySkinData->weights(vertexIndex, influenceIndex) = infuenceWeight;
		pySkinData->boneIDs(vertexIndex, influenceIndex) = influenceBoneID;
	}
}

PySkinData* SkinData::getData()
{
	if (!this->isValid)
	{
		throw std::exception("SkinData object is invalid. Cannot get skin weights.");
	}
	auto meshType = getMeshType(this->node);
	if (meshType == 0)
	{
		return this->getDataMesh();
	}
	else if (meshType == 1)
	{
		return this->getDataPoly();
	}

	throw std::exception("Unsupported mesh type, convert to EditablePoly or EditableMesh!");
}

PySkinData* SkinData::getDataPoly()
{
	unsigned int vertexCount = this->iSkinContextData->GetNumPoints();
	this->maxInfluenceCount = this->iSkinContextData->GetNumAssignedBones(0);
	PySkinData* pySkinData = new PySkinData(vertexCount, this->maxInfluenceCount);

	Matrix3 nodeTransform = this->node->GetObjectTM(0);
	bool deleteIt;
	PolyObject* polyObject = getPolyObjectFromNode(this->node, GetCOREInterface()->GetTime(), deleteIt);
	MNMesh& mnMesh = polyObject->GetMesh();
	for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		Point3 worldPosition = nodeTransform.PointTransform(mnMesh.V(vertexIndex)->p);
		pySkinData->positions(vertexIndex, 0) = worldPosition.x;
		pySkinData->positions(vertexIndex, 1) = worldPosition.y;
		pySkinData->positions(vertexIndex, 2) = worldPosition.z;
		this->collectWeightsAndBoneIDs(pySkinData, vertexIndex);
	};
	return pySkinData;
}

PySkinData* SkinData::getDataMesh()
{
	unsigned int vertexCount = this->iSkinContextData->GetNumPoints();
	this->maxInfluenceCount = this->iSkinContextData->GetNumAssignedBones(0);
	PySkinData* pySkinData = new PySkinData(vertexCount, this->maxInfluenceCount);

	Matrix3 nodeTransform = this->node->GetObjectTM(0);
	TriObject* triObject = getTriObjectFromNode(node, GetCOREInterface()->GetTime());
	Mesh& mesh = triObject->GetMesh();
	for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		Point3 worldPosition = nodeTransform.PointTransform(mesh.getVert(vertexIndex));
		pySkinData->positions(vertexIndex, 0) = worldPosition.x;
		pySkinData->positions(vertexIndex, 1) = worldPosition.y;
		pySkinData->positions(vertexIndex, 2) = worldPosition.z;
		this->collectWeightsAndBoneIDs(pySkinData, vertexIndex);
	};
	return pySkinData;
}


bool SkinData::setSkinWeights(Eigen::MatrixXi& boneIDs, Eigen::MatrixXf& vertexWeights)
{
	auto boneIDsRows = boneIDs.rows();
	auto vertexWeightsRows = vertexWeights.rows();
	auto boneIDsCols = boneIDs.cols();
	auto vertexWeightsCols = vertexWeights.cols();
	if (boneIDsRows != vertexWeightsRows) throw std::length_error(
		"skin bone ids count does not match skin weights count" + convertWCharToChar(this->node->GetName())
	);
	if (boneIDsCols != vertexWeightsCols) throw std::length_error(
		"skin bone ids count does not match skin weights count" + convertWCharToChar(this->node->GetName())
	);
	auto vertexCount = this->iSkinContextData->GetNumPoints();
	if (boneIDsRows != vertexCount) throw std::length_error(
		"skin vertex count does not match provided data count" + convertWCharToChar(this->node->GetName())
	);
	Tab<INode*> skinBones;
	auto skinBonesCount = this->iSkin->GetNumBones();
	for (auto boneIndex = 0; boneIndex < skinBonesCount; boneIndex++)
	{
		INode* bone = this->iSkin->GetBone(boneIndex);
		if (boneIndex == 0)
		{
			skinBones.Append(1, &bone, skinBonesCount);
			continue;
		}
		skinBones.Append(1, &bone);
	}
	for (auto vertexIndex = 0; vertexIndex < boneIDsRows; vertexIndex++)
	{
		Tab<INode*> bones = Tab<INode*>();
		Tab<float> weights = Tab<float>();
		bones.Resize(boneIDsCols);
		weights.Resize(boneIDsCols);
		for (auto influenceIndex = 0; influenceIndex < boneIDsCols; influenceIndex++)
		{
			auto boneID = boneIDs(vertexIndex, influenceIndex);
			auto influenceWeight = vertexWeights(vertexIndex, influenceIndex);
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


inline std::vector<std::vector<std::vector <float>>> getSkinWeights(wchar_t* name)
{
	SkinData skinData(name);
	return skinData.getSkinWeights();
}


bool setSkinWeights(wchar_t* name, Eigen::MatrixXi& boneIDs, Eigen::MatrixXf& weights)
{
	SkinData skinData(name);
	return skinData.setSkinWeights(boneIDs, weights);
}


//inline bool setSkinWeights(wchar_t* name, py::array boneIDs, py::array weights)
//{
//	SkinData* skinData = new SkinData(name);
//	return skinData->setSkinWeights(boneIDs, weights);
//}


//template <typename Sequence>
//inline py::array_t<typename Sequence::value_type> as_pyarray(Sequence&& seq) {
//	auto size = seq.size();
//	auto data = seq.data();
//	std::unique_ptr<Sequence> seq_ptr = std::make_unique<Sequence>(std::move(seq));
//	auto capsule = py::capsule(seq_ptr.get(), [](void* p) { std::unique_ptr<Sequence>(reinterpret_cast<Sequence*>(p)); });
//	seq_ptr.release();
//	return py::array(size, data, capsule);
//}
//
//
//template <typename Sequence>
//inline py::array_t<py::array_t<py::array_t<float>>> as_pyfloatarray(Sequence&& seq) {
//	auto size = seq.size();
//	auto data = seq.data();
//	std::unique_ptr<Sequence> seq_ptr = std::make_unique<Sequence>(std::move(seq));
//	auto capsule = py::capsule(seq_ptr.get(), [](void* p) { std::unique_ptr<Sequence>(reinterpret_cast<Sequence*>(p)); });
//	seq_ptr.release();
//	return py::array(size, data, capsule);
//}

//template <typename Sequence>
//inline py::array_t<typename Sequence::value_type> asNestedPyArray(Sequence&& seq) {
//
//}


PYBIND11_MODULE(SkinPlusPlusPymxs, m) {

	py::class_<PySkinData>(m, "SkinData")
		.def(py::init<int, int>())
		.def_readonly("bone_ids", &PySkinData::boneIDs)
		.def_readonly("weights", &PySkinData::weights)
		.def_readonly("positions", &PySkinData::positions)
		.def(
			py::pickle(
				[](const PySkinData& pySkinData) { // __getstate__
					return py::make_tuple(pySkinData.boneIDs, pySkinData.weights, pySkinData.positions);
				},
				[](py::tuple data) { // __setstate__
				
					PySkinData pySkinData(data);

					return pySkinData;
				}
			)
		)
		.def(
			"__repr__", [](const PySkinData& o) {
				auto vertexCount = o.boneIDs.rows();
				auto influenceCount = o.boneIDs.cols();
				return fmt::format("<PySkinData({}x{})>", vertexCount, influenceCount);
			}
		);
	m.def("get_skin_weights", [&](wchar_t* name, int return_type)
		{
			std::vector<std::vector<std::vector<float>>> weights = getSkinWeights(name);
			switch (return_type) {
				case 0:
					return py::cast(weights);
				case 1:
					return py::cast(weights, py::return_value_policy::take_ownership);
				case 2:
					return py::cast(weights, py::return_value_policy::copy);
				case 3:
					return py::cast(weights, py::return_value_policy::move);
				case 4:
					return py::cast(weights, py::return_value_policy::reference);
				case 5:
					return py::cast(weights, py::return_value_policy::reference_internal);
				case 6:
					return py::cast(weights, py::return_value_policy::automatic);
				case 7:
					return py::cast(weights, py::return_value_policy::automatic_reference);
				default:
					return py::cast(weights);
			}
			
		},
		"Get Skin Weights",
		py::arg("name"),
		py::arg("return_type")
	);
	m.def("get_data", [&](wchar_t* name)
		{
			SkinData skinData(name);
			PySkinData* pySkinData = skinData.getData();
			return pySkinData;
			
		},
		"Get Skin Data",
		py::arg("name")
	);
	m.def("set_skin_weights", [&](wchar_t* name, Eigen::MatrixXi& boneIDs, Eigen::MatrixXf& weights)
		{
			return setSkinWeights(name, boneIDs, weights);
		},
		"Set Skin Weights",
		py::arg("name"),
		py::arg("boneIDs"),
		py::arg("weights")
	);

	//m.def("__repr__", [](const PySkinData& pySkinData) {
	//	return "PySkinData<size:" + pySkinData.boneIDs.size() + "asdf>";
	//	}
	//);
}