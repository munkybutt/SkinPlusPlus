//#include "SkinPlusPlus.h"
//
//bool VertexData::appendBone(INode* inBone)
//{
//	return this->bones.Append(1, &inBone, 1);
//}
//
//bool VertexData::appendBoneID(Integer inBoneID)
//{
//	return this->boneIDs.Append(1, &inBoneID, 1);
//}
//
//bool VertexData::appendWeight(float inWeight)
//{
//	return this->weights.Append(1, &inWeight, 1);
//}
//
//
//SkinData::SkinData(INode* skinnedNode) { this->isValid = this->initialise(skinnedNode); }
//
//SkinData::~SkinData()
//{
//	this->allVertexData.~Tab();
//	this->cachedVertexData.~Tab();
//}
//
//INode* SkinData::getNode()
//{
//	return this->node;
//}
//
//bool SkinData::setData(INode* skinnedNode)
//{
//	this->allVertexData = Tab<VertexData*>();
//	this->node = skinnedNode;
//	Object* object = this->node->GetObjectRef();
//	if (!object || (object->SuperClassID() != GEN_DERIVOB_CLASS_ID))
//	{
//		return false;
//	}
//	this->derivedObject = (IDerivedObject*)(object);
//
//	for (int modifierIndex = 0; modifierIndex < this->derivedObject->NumModifiers(); modifierIndex++)
//	{
//		this->modifier = this->derivedObject->GetModifier(modifierIndex);
//
//		if (this->modifier->ClassID() != SKIN_CLASSID)
//		{
//			continue;
//		}
//		this->skin = (ISkin*)this->modifier->GetInterface(I_SKIN);
//
//		if (!this->skin)
//		{
//			continue;
//		}
//		this->skinImportData = (ISkinImportData*)this->modifier->GetInterface(I_SKINIMPORTDATA);
//
//		if (!this->skinImportData)
//		{
//			continue;
//		}
//		this->skinContextData = this->skin->GetContextInterface(skinnedNode);
//
//		if (!this->skinContextData)
//		{
//			continue;
//		}
//		this->boneModData = (BoneModData*)this->derivedObject->GetModContext(modifierIndex)->localData;
//
//		this->bonesDefMod = (BonesDefMod*)this->modifier;
//
//		return true;
//	}
//	return false;
//}
//
//void SkinData::cacheVertexDataForUndo()
//{
//	//theHold.Begin();
//	//if (theHold.Holding())
//	//{
//	//	theHold.Put(new SkinDataRestorObject(this));
//	//	theHold.Accept(_T("Caching Vertex Weights"));
//	//}
//	return;
//}
//
//VertexData* SkinData::setCachedVertexDataIO(const int inVertexIndex, Tab<INode*> inSkinBones)
//{
//	const int influenceCount = this->skinContextData->GetNumAssignedBones(inVertexIndex);
//	VertexData* vertexData = new VertexData(inVertexIndex);
//	for (int influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
//	{
//		const float vertexBoneWeight = this->skinContextData->GetBoneWeight(inVertexIndex, influenceIndex);
//		if (vertexBoneWeight < 0.00001f)
//		{
//			continue;
//		}
//		vertexData->appendWeight(vertexBoneWeight);
//		const int skinBoneIndex = this->skinContextData->GetAssignedBone(inVertexIndex, influenceIndex);
//		INode* skinBone = inSkinBones[skinBoneIndex];
//		vertexData->appendBone(skinBone);
//	}
//	return vertexData;
//}
//
//Tab<VertexData*> SkinData::setCachedVertexData()
//{
//	int vertexCount = this->skinContextData->GetNumPoints();
//	this->cachedVertexData = Tab<VertexData*>();
//	Tab<INode*> skinBones;
//	int skinBonesCount = this->skin->GetNumBones();
//	for (int boneIndex = 0; boneIndex < skinBonesCount; boneIndex++)
//	{
//		INode* bone = this->skin->GetBone(boneIndex);
//		skinBones.Append(1, &bone, 1);
//	}
//	for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
//	{
//		VertexData* vertexData = this->setCachedVertexDataIO(vertexIndex, skinBones);
//		this->cachedVertexData.Append(1, &vertexData, 1);
//	}
//	return this->cachedVertexData;
//}
//
//bool SkinData::setVertexDataFromCacheIO(VertexData* inVertexData)
//{
//	int id = inVertexData->getID();
//	Tab<INode*> bones = inVertexData->getBones();
//	Tab<float> weights = inVertexData->getWeights();
//	BOOL result = this->skinImportData->AddWeights(this->node, id, bones, weights);
//	if (result) return true; else return false;
//}
//
//bool SkinData::setVertexDataFromCache(Tab<VertexData*> inVertexDataTab)
//{
//	const int inVertexDataCount = inVertexDataTab.Count();
//	const int skinVertexCount = this->skinContextData->GetNumPoints();
//	if (inVertexDataCount != skinVertexCount)
//	{
//		throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), (Value*)this->node);
//	}
//	for (int vertexIndex = 0; vertexIndex < skinVertexCount; vertexIndex++)
//	{
//		VertexData* vertexData = inVertexDataTab[vertexIndex];
//		this->setVertexDataFromCacheIO(vertexData);
//	}
//	this->modifier->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
//	//GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
//	return true;
//}
//
//bool SkinData::setVertexDataIO(const int vertexIndex, Array* boneIDs, Array* vertexWeights, Tab<INode*> skinBones)
//{
//	Array* vertexBoneIDs = (Array*)boneIDs->data[vertexIndex];
//	Array* vertexBoneWeights = (Array*)vertexWeights->data[vertexIndex];
//	VertexData* vertexData = new VertexData(vertexIndex);
//	for (int vertexBoneIndex = 0; vertexBoneIndex < vertexBoneIDs->size; vertexBoneIndex++)
//	{
//		float weight = vertexBoneWeights->data[vertexBoneIndex]->to_float();
//		if (weight < 0.0f) weight = 0.0f;
//		if (weight > 1.0f) weight = 1.0f;
//		const int vertexBoneID = vertexBoneIDs->data[vertexBoneIndex]->to_int();
//		INode* vertexBoneIDNode = skinBones[vertexBoneID - 1];
//		vertexData->appendBone(vertexBoneIDNode);
//		vertexData->appendWeight(weight);
//	}
//	this->allVertexData.Append(1, &vertexData, 1);
//	BOOL result = this->skinImportData->AddWeights(this->node, vertexIndex, vertexData->getBones(), vertexData->getWeights());
//	if (result) return true; else return false;
//}
//
//bool SkinData::setAllVertexWeights(Array* boneIDs, Array* vertexWeights, Array* vertices)
//{
//	this->cacheVertexDataForUndo();
//	const int bonesArraySize = boneIDs->size;
//	const int weightsArraySize = vertexWeights->size;
//	if (bonesArraySize != weightsArraySize)
//	{
//		throw RuntimeError(GetString(IDS_PW_WEIGHT_BONE_COUNT), (Value*)this->node);
//	}
//	Tab<INode*> skinBones;
//	int skinBonesCount = this->skin->GetNumBones();
//	for (int boneIndex = 0; boneIndex < skinBonesCount; boneIndex++)
//	{
//		INode* bone = this->skin->GetBone(boneIndex);
//		skinBones.Append(1, &bone, 1);
//	}
//	bool allVertexWeightsSet = true;
//	if (vertices)
//	{
//		const int inVerticesCount = vertices->size;
//		for (int inVerticesIndex = 0; inVerticesIndex < inVerticesCount; inVerticesIndex++)
//		{
//			const int vertexIndex = vertices->data[inVerticesIndex]->to_int() - 1;
//			if (!this->setVertexDataIO(vertexIndex, boneIDs, vertexWeights, skinBones))
//			{
//				allVertexWeightsSet = false;
//			}
//		}
//	}
//	else
//	{
//		const int vertexCount = this->skinContextData->GetNumPoints();
//		for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
//		{
//			if (!this->setVertexDataIO(vertexIndex, boneIDs, vertexWeights, skinBones))
//			{
//				allVertexWeightsSet = false;
//			}
//		}
//	}
//
//	this->modifier->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
//	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
//	return allVertexWeightsSet;
//}
//
//bool SkinData::getVertexDataIO(const int inVertexIndex, VertexDataCollection* ioAllVertexData)
//{
//	const int influenceCount = this->skinContextData->GetNumAssignedBones(inVertexIndex);
//	Array* vertexBoneWeights = new Array(influenceCount);
//	Array* vertexBoneIDs = new Array(influenceCount);
//	for (int influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
//	{
//		const float vertexBoneWeight = this->skinContextData->GetBoneWeight(inVertexIndex, influenceIndex);
//		vertexBoneWeights->append(Double::intern(vertexBoneWeight));
//		const int skinBoneIndex = this->skinContextData->GetAssignedBone(inVertexIndex, influenceIndex) + 1;
//		vertexBoneIDs->append(Integer::intern(skinBoneIndex));
//	}
//	ioAllVertexData->weights->append(vertexBoneWeights);
//	ioAllVertexData->boneIDs->append(vertexBoneIDs);
//	return true;
//}
//
////BonesDefMod* SkinData::getBonesDefMod()
////{
////	if (!this->boneDefsMod)
////	{
////		SClass_ID sid = modifier->SuperClassID();
////		Class_ID id = modifier->ClassID();
////		if (sid != OSM_CLASS_ID || id != Class_ID(9815843, 87654))
////			return NULL;
////	}
////	this->boneDefsMod = (BonesDefMod*)modifier;
////}
//
//Array* SkinData::getVertexWeights()
//{
//	//int vertexCount = this->boneModData->VertexData.Count();
//	int vertexCount = this->skinContextData->GetNumPoints();
//
//	Array* outSkinData = new Array(2);
//	Array* weights = new Array(vertexCount);
//	Array* boneIDs = new Array(vertexCount);
//	outSkinData->data[0] = weights;
//	outSkinData->data[1] = boneIDs;
//
//	//the_listener->edit_stream->printf(_T("vertexCount: %d\n"), vertexCount);
//	for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
//	{
//		//this->bonesDefMod->GetNumBones();
//		const int influenceCount = this->skinContextData->GetNumAssignedBones(vertexIndex);
//		//const int influenceCount = this->boneModData->VertexData[vertexIndex]->WeightCount();
//		Array* influenceWeights = new Array(influenceCount);
//		Array* influenceBoneIDs = new Array(influenceCount);
//		for (int influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
//		{
//			const float infuenceWeight = this->skinContextData->GetBoneWeight(vertexIndex, influenceIndex);
//			//const float infuenceWeight = this->bonesDefMod->RetrieveNormalizedWeight(
//			//	this->boneModData,
//			//	vertexIndex,
//			//	influenceIndex
//			//);
//
//			//the_listener->edit_stream->printf(_T("Weight: %d\n"), infuenceWeight);
//			influenceWeights->append(Double::intern(infuenceWeight));
//			const int influenceBoneID = this->skinContextData->GetAssignedBone(vertexIndex, influenceIndex) + 1;
//			influenceBoneIDs->append(Integer::intern(influenceBoneID));
//			//the_listener->edit_stream->printf(_T("ID: %d\n"), influenceBoneID);
//			//the_listener->edit_stream->printf(_T("----------"));
//		}
//		weights->append(influenceWeights);
//		boneIDs->append(influenceBoneIDs);
//	}
//	outSkinData->data[0] = weights;
//	outSkinData->data[1] = boneIDs;
//
//	return outSkinData;
//}
//
//bool SkinData::getVertexWeights(int inVertexIndex, Array* ioWeightData, bool zeroBased)
//{
//	int vertexIndex = inVertexIndex;
//	if (!zeroBased) vertexIndex -= 1;
//	const int influenceCount = this->skinContextData->GetNumAssignedBones(vertexIndex);
//	Array* vertexWeightData = new Array(influenceCount);
//	Array* vertexBoneIDData = new Array(influenceCount);
//	for (int influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
//	{
//		const float vertexBoneWeight = this->skinContextData->GetBoneWeight(vertexIndex, influenceIndex);
//		vertexWeightData->append(Double::intern(vertexBoneWeight));
//		const int skinBoneIndex = this->skinContextData->GetAssignedBone(vertexIndex, influenceIndex) + 1;
//		vertexBoneIDData->append(Integer::intern(skinBoneIndex));
//	}
//	ioWeightData->append(vertexWeightData);
//	ioWeightData->append(vertexBoneIDData);
//
//	return true;
//}
//
//bool SkinData::setVertexWeights(int inVertexIndex, Array* inWeightData)
//{
//	//this->cacheVertexDataForUndo();
//	Array* vertexBoneWeights = (Array*)inWeightData->data[0];
//	Array* vertexBones = (Array*)inWeightData->data[1];
//	VertexData* vertexData = new VertexData(inVertexIndex);
//	for (int vertexBoneIndex = 0; vertexBoneIndex < vertexBoneWeights->size; vertexBoneIndex++)
//	{
//		float weight = vertexBoneWeights->data[vertexBoneIndex]->to_float();
//		if (weight < 0.0f) weight = 0.0f;
//		if (weight > 1.0f) weight = 1.0f;
//		MAXNode* mxsVertexBoneIDNode = (MAXNode*)vertexBones->data[vertexBoneIndex];
//		vertexData->appendBone(mxsVertexBoneIDNode->to_node());
//		vertexData->appendWeight(weight);
//	}
//	this->allVertexData.Append(1, &vertexData, 1);
//	BOOL result = this->skinImportData->AddWeights(
//		this->node,
//		inVertexIndex,
//		vertexData->getBones(),
//		vertexData->getWeights()
//	);
//
//	this->modifier->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
//	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
//
//	if (result) return true; else return false;
//}
//
//int SkinData::getBoneID(INode* inBone, bool zeroBased)
//{
//	const int boneCount = this->skin->GetNumBones();
//	for (int boneID = 0; boneID < boneCount; boneID++)
//	{
//		INode* bone = this->skin->GetBone(boneID);
//		if (bone == inBone)
//		{
//			if (zeroBased)
//			{
//				return boneID;
//			}
//			return boneID + 1;
//		}
//	}
//	return -1;
//}
//
//bool SkinData::replaceBoneWeightsIO(int vertexIndex, INode* newBone, int oldBoneID, int newBoneID, Tab<INode*> skinnedBones)
//{
//	int oldBoneIDFound = false;
//	bool newBoneIDFound = false;
//	float totalVertexBoneWeight = 0.0;
//	Tab<INode*> vertexBonesTab;
//	Tab<float> vertexBoneWeightsTab;
//	const int influenceCount = this->skinContextData->GetNumAssignedBones(vertexIndex);
//	for (int influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
//	{
//		float vertexBoneWeight = this->skinContextData->GetBoneWeight(vertexIndex, influenceIndex);
//		const int skinBoneIndex = this->skinContextData->GetAssignedBone(vertexIndex, influenceIndex);
//		if (oldBoneID == skinBoneIndex)
//		{
//			oldBoneIDFound = true;
//			totalVertexBoneWeight += vertexBoneWeight;
//		}
//		else if (newBoneID == skinBoneIndex)
//		{
//			newBoneIDFound = true;
//			totalVertexBoneWeight += vertexBoneWeight;
//		}
//		else
//		{
//			INode* bone = skinnedBones[skinBoneIndex];
//			vertexBonesTab.Append(1, &bone, 1);
//			vertexBoneWeightsTab.Append(1, &vertexBoneWeight, 1);
//		}
//	}
//	if (totalVertexBoneWeight > 0.0)
//	{
//		vertexBonesTab.Append(1, &newBone, 1);
//		vertexBoneWeightsTab.Append(1, &totalVertexBoneWeight, 1);
//		BOOL mxsResult = this->skinImportData->AddWeights(this->node, vertexIndex, vertexBonesTab, vertexBoneWeightsTab);
//		if (!mxsResult)
//		{
//			return false;
//		}
//	}
//	return true;
//}
//
//bool SkinData::replaceBoneWeights(INode* oldBone, INode* newBone, Array* inVertexIDs)
//{
//
//	if (!this->skinContextData)
//	{
//		return false;
//	}
//
//	int oldBoneID = this->getBoneID(oldBone, true);
//	if (oldBoneID == -1)
//	{
//		throw RuntimeError(_T("No bone id found,make sure it exists in the skin modifier: "), oldBone->GetName());
//	}
//
//	int newBoneID = this->getBoneID(newBone, true);
//	if (newBoneID == -1)
//	{
//		throw RuntimeError(_T("No bone id found, make sure it exists in the skin modifier: "), newBone->GetName());
//	}
//	//this->cacheVertexDataForUndo();
//
//	int skinBonesCount = this->skin->GetNumBones();
//	Tab<INode*> skinBones;
//
//	for (int boneIndex = 0; boneIndex < skinBonesCount; boneIndex++)
//	{
//		INode* bone = this->skin->GetBone(boneIndex);
//		skinBones.Append(1, &bone, 1);
//	}
//
//	bool result = true;
//	if (inVertexIDs)
//	{
//		for (int index = 0; index < inVertexIDs->size; index++)
//		{
//			int vertexIndex = inVertexIDs->data[index]->to_int() - 1;
//			result = this->replaceBoneWeightsIO(vertexIndex, newBone, oldBoneID, newBoneID, skinBones);
//			if (!result)
//			{
//				the_listener->edit_stream->printf(_T("Failed to set weight for vertex: %d\n"), vertexIndex);
//				break;
//			}
//		}
//	}
//	else
//	{
//		int vertexCount = this->skinContextData->GetNumPoints();
//		for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
//		{
//			result = this->replaceBoneWeightsIO(vertexIndex, newBone, oldBoneID, newBoneID, skinBones);
//			if (!result)
//			{
//				the_listener->edit_stream->printf(_T("Failed to set weight for vertex: %d\n"), vertexIndex);
//				break;
//			}
//		}
//	}
//	this->modifier->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
//	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
//
//	return result;
//}
//
//
//
//Array* getVertexWeights(INode* node)
//{
//	SkinData* skinData = new SkinData(node);
//	return skinData->getVertexWeights();
//}
//
//bool setVertexWeights(INode* node, Array* boneIDs, Array* weights)
//{
//	SkinData* skinData = new SkinData(node);
//	return skinData->setAllVertexWeights(boneIDs, weights);
//}
//
//
//
//static Modifier* getSkinModifier(INode* node)
//{
//	Object* obj = node->GetObjectRef();
//	if (!obj || (obj->SuperClassID() != GEN_DERIVOB_CLASS_ID))
//	{
//		return NULL;
//	}
//
//	IDerivedObject* derivedObject = (IDerivedObject*)(obj);
//	for (int modifierIndex = 0; modifierIndex < derivedObject->NumModifiers(); modifierIndex++)
//	{
//		Modifier* modifier = derivedObject->GetModifier(modifierIndex);
//		if (modifier->ClassID() != SKIN_CLASSID)
//		{
//			continue;
//		}
//
//		if ((ISkin*)modifier->GetInterface(I_SKIN))
//		{
//			return modifier;
//		}
//	}
//	return NULL;
//}
//
//static ISkinContextData* getSkinContextData(INode* inSkinnedNode)
//{
//	Object* obj = inSkinnedNode->GetObjectRef();
//
//	if (!obj || (obj->SuperClassID() != GEN_DERIVOB_CLASS_ID))
//	{
//		return NULL;
//	}
//
//	IDerivedObject* derObj = (IDerivedObject*)(obj);
//
//	for (int modifierIndex = 0; modifierIndex < derObj->NumModifiers(); modifierIndex++)
//	{
//		Modifier* modifier = derObj->GetModifier(modifierIndex);
//		if (modifier->ClassID() != SKIN_CLASSID)
//		{
//			continue;
//		}
//
//		ISkin* skin = (ISkin*)modifier->GetInterface(I_SKIN);
//
//		if (skin == NULL)
//		{
//			continue;
//		}
//
//		ISkinContextData* skinContextData = skin->GetContextInterface(inSkinnedNode);
//		if (!skinContextData)
//		{
//			return NULL;
//		}
//
//		return skinContextData;
//	}
//
//	return NULL;
//}
//
//static ISkinImportData* getSkinImportData(INode* inSkinnedNode)
//{
//	Object* obj = inSkinnedNode->GetObjectRef();
//
//	if (!obj || (obj->SuperClassID() != GEN_DERIVOB_CLASS_ID))
//	{
//		return NULL;
//	}
//
//	IDerivedObject* derObj = (IDerivedObject*)(obj);
//
//	for (int modifierIndex = 0; modifierIndex < derObj->NumModifiers(); modifierIndex++)
//	{
//		Modifier* modifier = derObj->GetModifier(modifierIndex);
//
//		if (modifier->ClassID() != SKIN_CLASSID)
//		{
//			continue;
//		}
//
//		ISkinImportData* skinImportData = (ISkinImportData*)modifier->GetInterface(I_SKINIMPORTDATA);
//
//		if (skinImportData == NULL)
//		{
//			continue;
//		}
//
//		return skinImportData;
//	}
//
//	return NULL;
//}
