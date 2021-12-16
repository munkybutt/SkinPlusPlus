//**************************************************************************/
// Copyright (c) 1998-2021 Autodesk, Inc.
// All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "SkinPlusPlus.h"

#define SkinPlusPlus_CLASS_ID	Class_ID(0x5b374d7b, 0x68620ce)


class SkinPlusPlus : public GUP
{
public:
	//Constructor/Destructor
	SkinPlusPlus();
	virtual ~SkinPlusPlus();

	// GUP Methods
	virtual DWORD     Start();
	virtual void      Stop();
	virtual DWORD_PTR Control(DWORD parameter);
	virtual void      DeleteThis();

	// Loading/Saving
	virtual IOResult Save(ISave* isave);
	virtual IOResult Load(ILoad* iload);
};

class SkinPlusPlusClassDesc : public ClassDesc2 
{
public:
	virtual int           IsPublic() override                       { return TRUE; }
	virtual void*         Create(BOOL /*loading = FALSE*/) override { return new SkinPlusPlus(); }
	virtual const TCHAR * ClassName() override                      { return GetString(IDS_CLASS_NAME); }
	virtual const MCHAR*  NonLocalizedClassName() override			{ return GetString(IDS_CLASS_NAME); }
	
	virtual SClass_ID     SuperClassID() override                   { return GUP_CLASS_ID; }
	virtual Class_ID      ClassID() override                        { return SkinPlusPlus_CLASS_ID; }
	virtual const TCHAR*  Category() override                       { return GetString(IDS_CATEGORY); }

	virtual const TCHAR*  InternalName() override                   { return _T("SkinPlusPlus"); } // Returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE     HInstance() override                      { return hInstance; } // Returns owning module handle


};

ClassDesc2* GetSkinPlusPlusDesc()
{
	static SkinPlusPlusClassDesc SkinPlusPlusDesc;
	return &SkinPlusPlusDesc; 
}

SkinPlusPlus::SkinPlusPlus()
{

}

SkinPlusPlus::~SkinPlusPlus()
{

}

void SkinPlusPlus::DeleteThis()
{
	delete this;
}

// Activate and Stay Resident
DWORD SkinPlusPlus::Start()
{
	return GUPRESULT_KEEP;
}

void SkinPlusPlus::Stop()
{
}

DWORD_PTR SkinPlusPlus::Control( DWORD /*parameter*/ )
{
	return 0;
}

IOResult SkinPlusPlus::Save(ISave* /*isave*/)
{
	return IO_OK;
}

IOResult SkinPlusPlus::Load(ILoad* /*iload*/)
{
	return IO_OK;
}


// VertexData Methods:

VertexData::VertexData(int vertexID, Array* boneIDs, Array* weights, Tab<INode*> skinBones)
{
	if (!(boneIDs->size == weights->size))
		throw RuntimeError(_T("ids count does not match weights count"), Integer::intern(vertexID));

	this->initialiseVariables(boneIDs->size);
	float weightCap = 1.0f;
	for (int vertexBoneIndex = 0; vertexBoneIndex < boneIDs->size; vertexBoneIndex++)
	{
		float weight = weights->data[vertexBoneIndex]->to_float();
		const int vertexBoneID = boneIDs->data[vertexBoneIndex]->to_int();
		INode* vertexBoneIDNode = skinBones[vertexBoneID - 1];
		if (weight < 0.0f) continue;
		this->bones.Append(1, &vertexBoneIDNode);
		if (weight > 1.0f) this->weights.Append(1, &weightCap); else this->weights.Append(1, &weight);
	}
};

void VertexData::initialiseVariables(int size)
{
	this->bones = Tab<INode*>();
	this->weights = Tab<float>();
	this->bones.Resize(size);
	this->weights.Resize(size);

}

void VertexData::appendVariables(INode* bone, float weight)
{
	this->bones.Append(1, &bone);
	this->weights.Append(1, &weight);
}


PolyObject* getPolyObjectFromNode(INode* inNode, TimeValue inTime, bool& deleteIt)
{
	Object* object = inNode->GetObjectRef();
	Object* nodeObjectBase = object->FindBaseObject();
	auto classID = Class_ID(POLYOBJ_CLASS_ID, 0);
	if (nodeObjectBase->CanConvertToType(classID))
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


 //SkinData Methods:
bool SkinData::initialise(INode* skinnedNode)
{
	this->allVertexData = Tab<VertexData*>();
	this->node = skinnedNode;
	Object* object = this->node->GetObjectRef();
	if (!object || (object->SuperClassID() != GEN_DERIVOB_CLASS_ID))
	{
		return false;
	}
	this->iDerivedObject = (IDerivedObject*)(object);
	if (!this->iDerivedObject) return false;
	
	for (int modifierIndex = 0; modifierIndex < this->iDerivedObject->NumModifiers(); modifierIndex++)
	{
		this->skinModifier = this->iDerivedObject->GetModifier(modifierIndex);
		if (this->skinModifier->ClassID() != SKIN_CLASSID) continue;

		this->iSkin = (ISkin*)this->skinModifier->GetInterface(I_SKIN);
		if (!this->iSkin) continue;

		this->iSkinContextData = this->iSkin->GetContextInterface(skinnedNode);
		if (!this->iSkinContextData) continue;

		this->iSkinImportData = (ISkinImportData*)this->skinModifier->GetInterface(I_SKINIMPORTDATA);
		if (!this->iSkinImportData) continue;

		return true;
	}
	return false;
}


void SkinData::getSkinWeights(Array& ioSkinData)
{
	int vertexCount = this->iSkinContextData->GetNumPoints();
	ScopedMaxScriptEvaluationContext scopedMaxScriptEvaluationContext;
	MAXScript_TLS* _tls = scopedMaxScriptEvaluationContext.Get_TLS();
	seven_typed_value_locals_tls(
		Array* skinData,
		Array* weights,
		Array* positions,
		Array* boneIDs,
		Array* influenceWeights,
		Array* influenceBoneIDs,
		Array* position
	);
	vl.weights = new Array(vertexCount);
	vl.boneIDs = new Array(vertexCount);
	vl.positions = new Array(vertexCount);
	vl.weights->size = vertexCount;
	vl.boneIDs->size = vertexCount;
	vl.positions->size = vertexCount;
	Matrix3 nodeTransform = this->node->GetObjectTM(0);
	Object* nodeBaseObject = this->node->GetObjectRef()->FindBaseObject();

	//if (nodeBaseObject->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0)))
	EPoly* ePolyInterface = (EPoly*)(nodeBaseObject->GetInterface(EPOLY_INTERFACE));
	MNMesh* mnMesh = ePolyInterface->GetMeshPtr();
	for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		int influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
		vl.influenceWeights = new Array(influenceCount);
		vl.influenceBoneIDs = new Array(influenceCount);
		vl.influenceWeights->size = influenceCount;
		vl.influenceBoneIDs->size = influenceCount;
		for (int influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
		{
			float infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
			int influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex) + 1;
			vl.influenceWeights->data[influenceIndex] = Float::intern(infuenceWeight);
			vl.influenceBoneIDs->data[influenceIndex] = Integer::intern(influenceBoneID);
		}
		MNVert vertex = mnMesh->v[vertexIndex];
		Point3 vertexPosition = vertex.p;
		Point3 worldPosition = nodeTransform.PointTransform(vertexPosition);

		vl.position = new Array(3);
		vl.position->size = 3;
		vl.position->data[0] = Float::intern(worldPosition.x);
		vl.position->data[1] = Float::intern(worldPosition.y);
		vl.position->data[2] = Float::intern(worldPosition.z);

		vl.weights->data[vertexIndex] = vl.influenceWeights;
		vl.boneIDs->data[vertexIndex] = vl.influenceBoneIDs;
		vl.positions->data[vertexIndex] = vl.position;
	};
	ioSkinData.data[0] = vl.weights;
	ioSkinData.data[1] = vl.boneIDs;
	ioSkinData.data[2] = vl.positions;
}


Array* SkinData::getSkinWeights()
{
	int vertexCount = this->iSkinContextData->GetNumPoints();
	ScopedMaxScriptEvaluationContext scopedMaxScriptEvaluationContext;
	MAXScript_TLS* _tls = scopedMaxScriptEvaluationContext.Get_TLS();
	seven_typed_value_locals_tls(
		Array* skinData,
		Array* weights,
		Array* positions,
		Array* boneIDs,
		Array* influenceWeights,
		Array* influenceBoneIDs,
		Array* position
	);
	vl.weights = new Array(vertexCount);
	vl.boneIDs = new Array(vertexCount);
	vl.positions = new Array(vertexCount);
	vl.weights->size = vertexCount;
	vl.boneIDs->size = vertexCount;
	vl.positions->size = vertexCount;

	Matrix3 nodeTransform = this->node->GetObjectTM(0);
	bool deleteIt;
	PolyObject* polyObject = getPolyObjectFromNode(this->node, GetCOREInterface()->GetTime(), deleteIt);
	MNMesh& mnMesh = polyObject->GetMesh();

	for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		the_listener->edit_stream->printf(_T("vertexIndex: %d"), vertexIndex);
		Point3 worldPosition = nodeTransform.PointTransform(mnMesh.V(vertexIndex)->p);
		vl.position = new Array(3);
		vl.position->size = 3;
		vl.position->data[0] = Float::intern(0.0f);
		vl.position->data[1] = Float::intern(0.0f);
		vl.position->data[2] = Float::intern(0.0f);
		int influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
		vl.influenceWeights = new Array(influenceCount);
		vl.influenceBoneIDs = new Array(influenceCount);
		vl.influenceWeights->size = influenceCount;
		vl.influenceBoneIDs->size = influenceCount;
		for (int influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
		{
			float infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
			int influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex) + 1;
			vl.influenceWeights->data[influenceIndex] = Float::intern(infuenceWeight);
			vl.influenceBoneIDs->data[influenceIndex] = Integer::intern(influenceBoneID);
		}
		vl.weights->data[vertexIndex] = vl.influenceWeights;
		vl.boneIDs->data[vertexIndex] = vl.influenceBoneIDs;
		vl.positions->data[vertexIndex] = vl.position;
	};
	vl.skinData = new Array(3);
	vl.skinData->size = 3;
	vl.skinData->data[0] = vl.weights;
	vl.skinData->data[1] = vl.boneIDs;
	vl.skinData->data[2] = vl.positions;
	return_value_tls(vl.weights);
}


bool SkinData::setVertexWeights(int vertexIndex, Array* vertexBones, Array* vertexWeights)
{
	if (!(vertexWeights->size == vertexBones->size))
		throw RuntimeError(_T("ids count does not match weights count"), (Value*)this->node);

	VertexData* vertexData = new VertexData(vertexWeights->size);
	for (int vertexBoneIndex = 0; vertexBoneIndex < vertexWeights->size; vertexBoneIndex++)
	{
		float weight = vertexWeights->data[vertexBoneIndex]->to_float();
		if (weight <= 0.0f) continue;
		if (weight > 1.0f) weight = 1.0f;
		MAXNode* mxsVertexBoneIDNode = (MAXNode*)vertexBones->data[vertexBoneIndex];
		vertexData->appendVariables(mxsVertexBoneIDNode->to_node(), weight);
	}
	bool result = this->iSkinImportData->AddWeights(this->node, vertexIndex, vertexData->getBones(), vertexData->getWeights());
	this->skinModifier->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	return result;
}


bool SkinData::setVertexWeights(const int vertexIndex, Array* boneIDs, Array* vertexWeights, Tab<INode*> skinBones)
{
	Array* vertexBoneIDs = (Array*)boneIDs->data[vertexIndex];
	Array* vertexBoneWeights = (Array*)vertexWeights->data[vertexIndex];
	VertexData* vertexData = new VertexData(vertexIndex, vertexBoneIDs, vertexBoneWeights, skinBones);
	return this->iSkinImportData->AddWeights(this->node, vertexIndex, vertexData->getBones(), vertexData->getWeights());
}


bool SkinData::setSkinWeights(Array* boneIDs, Array* vertexWeights, Array* vertices)
{
	const int bonesArraySize = boneIDs->size;
	const int weightsArraySize = vertexWeights->size;
	if (bonesArraySize != weightsArraySize) throw RuntimeError(
		_T("skin bone ids count does not match skin weights count"), (Value*)this->node
	);
	Tab<INode*> skinBones;
	int skinBonesCount = this->iSkin->GetNumBones();
	for (int boneIndex = 0; boneIndex < skinBonesCount; boneIndex++)
	{
		INode* bone = this->iSkin->GetBone(boneIndex);
		if (boneIndex == 0)
		{
			skinBones.Append(1, &bone, skinBonesCount);
			continue;
		}
		skinBones.Append(1, &bone);
	}
	bool skinWeightsSet = true;
	const int vertexCount = this->iSkinContextData->GetNumPoints();
	for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		Array* vertexBoneIDs = (Array*)boneIDs->data[vertexIndex];
		Array* vertexBoneWeights = (Array*)vertexWeights->data[vertexIndex];
		VertexData* vertexData = new VertexData(vertexIndex, vertexBoneIDs, vertexBoneWeights, skinBones);
		if (!(this->iSkinImportData->AddWeights(this->node, vertexIndex, vertexData->getBones(), vertexData->getWeights())))
			skinWeightsSet = false;
	}

	this->skinModifier->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	return skinWeightsSet;
}


// SkinData Functions:
void getSkinWeightsFn(INode* node, Array& ioSkinData)
{
	SkinData* skinData = new SkinData(node);
	skinData->getSkinWeights(ioSkinData);
}


Array* getSkinWeightsFn(INode* node)
{
	SkinData* skinData = new SkinData(node);
	return skinData->getSkinWeights();
}


bool setVertexWeights(INode* node, int vertexIndex, Array* weightData)
{
	SkinData* skinData = new SkinData(node);
	return skinData->setVertexWeights(vertexIndex, (Array*)weightData->data[0], (Array*)weightData->data[1]);
}


bool setSkinWeightsFn(INode* node, Array* boneIDs, Array* vertexWeights, Array* vertices)
{
	SkinData* skinData = new SkinData(node);
	return skinData->setSkinWeights(boneIDs, vertexWeights, vertices);
}


Value* getSkinWeightsIO(Value** args)
{
	type_check(args[0], MAXNode, _T("getSkinWeights"));
	one_typed_value_local(Array* skinData);
	vl.skinData = new Array(3);
	vl.skinData->size = 3;
	getSkinWeightsFn(((MAXNode*)args[0])->to_node(), *vl.skinData);
	return_value(vl.skinData);
}


Value* setSkinWeightsIO(Value** args)
{
	type_check(args[0], MAXNode, _T("setSkinWeights"));
	type_check(args[1], Array, _T("setSkinWeights"));
	type_check(args[2], Array, _T("setSkinWeights"));
	type_check(args[3], Array, _T("setSkinWeights"));
	bool result = setSkinWeightsFn(((MAXNode*)args[0])->to_node(), (Array*)args[1], (Array*)args[2], (Array*)args[3]);
	if (result)
	{
		return &true_value;
	}
	return &false_value;
}


Value* getSkinWeightsMethod_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getSkinWeightsMethod, 1, count);
	return getSkinWeightsIO(arg_list);
}

Value* getSkinWeightsFunction_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getSkinWeightsFunction, 1, count);
	return getSkinWeightsIO(arg_list);
}

def_struct_primitive_debug_ok(getSkinWeightsMethod, SkinPPOps, "GetSkinWeights");

def_visible_primitive_debug_ok(getSkinWeightsFunction, "SPPGetSkinWeights");


Value* setSkinWeightsMethod_cf(Value** arg_list, int count)
{
	// INode* skinnedNode, Value* boneIDs, Value* vertexWeights, Value* vertices
	check_arg_count_with_keys(setSkinWeightsMethod, 4, count);
	return setSkinWeightsIO(arg_list);
}

Value* setSkinWeightsFunction_cf(Value** arg_list, int count)
{
	// INode* skinnedNode, Value* boneIDs, Value* vertexWeights, Value* vertices
	check_arg_count_with_keys(setSkinWeightsFunction, 4, count);
	return setSkinWeightsIO(arg_list);
}

def_struct_primitive(setSkinWeightsMethod, SkinPPOps, "SetSkinWeights");

def_visible_primitive(setSkinWeightsFunction, "SPPSetSkinWeights");


static bool applyOffsetToVertices(INode* inNode, Point3 inOffset)
{
	static bool success = false;
	bool deleteIt = false;
	bool polyDeleteIt = false;
	Interface* coreInterface = GetCOREInterface();
	PolyObject* polyObject = getPolyObjectFromNode(inNode, coreInterface->GetTime(), deleteIt);
	if (polyObject)
	{
		MNMesh& mesh = polyObject->GetMesh();
		for (int vertIndex = 0; vertIndex < mesh.VNum(); vertIndex++)
		{
			MNVert* vert = mesh.V(vertIndex);
			vert->p += inOffset;
		}
		inNode->SetObjectRef(polyObject);
		polyObject->FreeCaches();
		polyObject->NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);
		coreInterface->RedrawViews(coreInterface->GetTime());
		success = true;
	}
	return success;
}

Value* getPositions(INode* inNode)
{
	Object* nodeBaseObject = inNode->GetObjectRef()->FindBaseObject();
	if (nodeBaseObject->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0)))
	{
		EPoly* ePolyInterface = (EPoly*)(nodeBaseObject->GetInterface(EPOLY_INTERFACE));
		MNMesh* mnMesh = ePolyInterface->GetMeshPtr();
		Array* positions = new Array(mnMesh->VNum());
		positions->size = mnMesh->VNum();
		for (int index = 0; index < mnMesh->VNum(); index++)
		{
			MNVert vertex = mnMesh->v[0];
			Point3 vertexPosition = vertex.p;
			Array* _position = new Array(3);
			_position->size = 3;
			_position->data[0] = Float::intern(vertexPosition.x);
			_position->data[1] = Float::intern(vertexPosition.y);
			_position->data[2] = Float::intern(vertexPosition.z);
			positions->data[index] = _position;
		}

		return positions;
	}
	return &Undefined_class;
}

Value* getPositions_cf(Value** arg_list, int count)
{
	// INode* skinnedNode, Value* boneIDs, Value* vertexWeights, Value* vertices
	check_arg_count_with_keys(getPositions, 1, count);
	INode* node = ((MAXNode*)arg_list[0])->to_node();
	return getPositions(node);
}

def_struct_primitive(getPositions, SkinPPOps, "getPositions");


struct SkinPlusPlusFunctionPublish : public FPStaticInterface
{
	DECLARE_DESCRIPTOR(SkinPlusPlusFunctionPublish);

	enum FunctionIDs
	{
		ID_GetSkinWeights,
		ID_SetVertexWeights,
		ID_SetSkinWeights,
		ID_ReplaceBoneWeights,
	};
	BEGIN_FUNCTION_MAP
	FN_1(ID_GetSkinWeights, TYPE_VALUE, Fn_GetSkinWeights, TYPE_INODE);
	FN_3(ID_SetVertexWeights, TYPE_BOOL, Fn_SetVertexWeights, TYPE_INODE, TYPE_INT, TYPE_VALUE);
	FN_4(ID_SetSkinWeights, TYPE_BOOL, Fn_SetSkinWeights, TYPE_INODE, TYPE_VALUE, TYPE_VALUE, TYPE_VALUE);
	//FN_4(ID_SetVertexWeights, TYPE_VALUE, Fn_SetVertexWeights, TYPE_VALUE, TYPE_VALUE, TYPE_VALUE, TYPE_VALUE);
	//FN_4(ID_ReplaceBoneWeights, TYPE_VALUE, Fn_ReplaceBoneWeights, TYPE_INODE, TYPE_INODE, TYPE_INODE, TYPE_VALUE);
	END_FUNCTION_MAP

	const Array* Fn_GetSkinWeights(INode* skinnedNode)
	{
		one_typed_value_local(Array* skinData);
		//vl.skinData = new Array(2);
		//vl.skinData->size = 2;
		//getSkinWeightsFn(skinnedNode, *vl.skinData);
		vl.skinData = getSkinWeightsFn(skinnedNode);
		return_value(vl.skinData);
	}
	
	const BOOL Fn_SetVertexWeights(INode* skinnedNode, int vertexIndex, Value* weightData)
	{
		Array* _weightData = (Array*)weightData;
		return setVertexWeights(skinnedNode, vertexIndex, _weightData);
	}
	
	const BOOL Fn_SetSkinWeights(INode* skinnedNode, Value* boneIDs, Value* vertexWeights, Value* vertices)
	{
		type_check(boneIDs, Array, _T("SetSkinWeights"));
		type_check(vertexWeights, Array, _T("SetSkinWeights"));
		Array* vertices_ = NULL;
		if (vertices && vertices != &undefined)
		{
			type_check(vertices, Array, _T("SetSkinWeights"));
			vertices_ = (Array*)vertices;
		}
		return setSkinWeightsFn(skinnedNode, (Array*)boneIDs, (Array*)vertexWeights, vertices_);
	}


	//const Value* Fn_ReplaceBoneWeights(
	//	INode* skinnedNode,
	//	INode* oldBone,
	//	INode* newBone,
	//	Value* verticesArray = &undefined
	//)
	//{
	//	SkinData* skinData = new SkinData(skinnedNode);
	//	Array* verticesArray_ = NULL;
	//	if (verticesArray && verticesArray != &undefined)
	//	{
	//		type_check(verticesArray, Array, _T("ReplaceBoneWeights"));
	//		verticesArray_ = (Array*)verticesArray;
	//	}
	//	if (skinData->replaceBoneWeights(oldBone, newBone, verticesArray_))
	//	{
	//		return &true_value;
	//	}
	//	return &false_value;
	//}

	//const Value* Fn_SetVertexWeights(
	//	Value* skinnedNode,
	//	Value* vertexBoneIDsArray,
	//	Value* vertexWeightsArray,
	//	Value* verticesArray = &undefined
	//)
	//{
	//	type_check(skinnedNode, MAXNode, _T("SetVertexWeight"));
	//	type_check(vertexBoneIDsArray, Array, _T("SetVertexWeight"));
	//	type_check(vertexWeightsArray, Array, _T("SetVertexWeight"));
	//	Array* verticesArray_ = NULL;
	//	if (verticesArray != &undefined)
	//	{
	//		verticesArray_ = (Array*)verticesArray;
	//	}
	//	if (setVertexWeights(skinnedNode->to_node(), (Array*)vertexBoneIDsArray, (Array*)vertexWeightsArray))
	//	{
	//		return &true_value;
	//	}
	//	return &false_value;
	//}

};


static SkinPlusPlusFunctionPublish skinPlusPlusFunctionPublish
(
	Interface_ID(0x231958ad, 0x1ea823e8),
	_T("SkinPP"),
	-1,
	0,
	FP_CORE,
	SkinPlusPlusFunctionPublish::ID_GetSkinWeights, _T("GetSkinWeights"), 0, TYPE_VALUE, 0, 1,
	_T("skinnedNode"), 0, TYPE_INODE,
	SkinPlusPlusFunctionPublish::ID_SetVertexWeights, _T("SetVertexWeights"), 0, TYPE_VALUE, 0, 3,
	_T("skinnedNode"), 0, TYPE_INODE,
	_T("vertexIndex"), 0, TYPE_INT,
	_T("weightData"), 0, TYPE_VALUE,
	SkinPlusPlusFunctionPublish::ID_SetSkinWeights, _T("SetSkinWeights"), 0, TYPE_VALUE, 0, 4,
	_T("skinnedNode"), 0, TYPE_INODE,
	_T("vertBoneIDsArray"), 0, TYPE_VALUE,
	_T("weightsArray"), 0, TYPE_VALUE,
	_T("verticesArray"), 0, TYPE_VALUE, f_keyArgDefault, NULL,
	//SkinPlusPlusFunctionPublish::ID_ReplaceBoneWeights, _T("ReplaceBoneWeights"), 0, TYPE_VALUE, 0, 4,
	//_T("skinnedNode"), 0, TYPE_INODE,
	//_T("oldBone"), 0, TYPE_INODE,
	//_T("newBone"), 0, TYPE_INODE,
	//_T("vertexIDs"), 0, TYPE_VALUE, f_keyArgDefault, NULL,
	p_end
);

