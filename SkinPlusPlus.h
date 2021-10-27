#pragma once


#include "3dsmaxsdk_preinclude.h"
#include "resource.h"
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <maxtypes.h>
#include <guplib.h>
#include "modstack.h"
//#include "modifiers/bonesdef/BONESDEF.H"
#include <maxscript/maxwrapper/mxsobjects.h>
#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include "maxscript/macros/define_external_functions.h"
#include <maxscript/macros/define_instantiation_functions.h>
#include <iskin.h>


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

// A struct containing weight data of a single skinned vertex.
struct VertexData
{
private:
	Tab<INode*> bones;
	Tab<float> weights;

public:
	VertexData(int size) { this->initialiseVariables(size); };
	VertexData(int vertexID, Array* boneIDs, Array* weights, Tab<INode*> skinBones);
	void initialiseVariables(int size);
	void appendVariables(INode* bone, float weight);
	Tab<INode*> getBones() { return this->bones; };
	Tab<float> getWeights() { return this->weights; };
};

 //A struct containing weight data of all skinned vertices.
struct VertexDataCollection
{
public:
	Array* weights;
	Array* boneIDs;

	VertexDataCollection() {};
	VertexDataCollection(int vertexCount)
	{
		this->weights = new Array(vertexCount);
		this->boneIDs = new Array(vertexCount);
	};
};

class SkinData
{
protected:
	// Properties:
	bool isValid = false;
	INode* node;
	IDerivedObject* iDerivedObject;
	Modifier* skinModifier;
	ISkin* iSkin;
	ISkinContextData* iSkinContextData; // used to query skin data
	ISkinImportData* iSkinImportData; // used to modify skin data
	Tab<VertexData*> allVertexData;
	Tab<VertexData*> cachedUndoVertexData;

public:
	SkinData() {};
	SkinData(INode* skinnedNode) { this->isValid = this->initialise(skinnedNode); }
	~SkinData()
	{
		this->allVertexData.~Tab();
		this->cachedUndoVertexData.~Tab();
	}
	bool initialise(INode* skinnedNode);
	Tab<VertexData*> getAllVertexData() { return this->allVertexData; };
	Tab<VertexData*> getCachedVertexData() { return this->cachedUndoVertexData; };
	virtual void getSkinWeights(Array& ioSkinData);
	virtual Array* getSkinWeights();
	bool setSkinWeights(Array* boneIDs, Array* vertexWeights, Array* vertices = NULL);
	bool setVertexWeights(int vertexIndex, Array* vertexBones, Array* vertexWeights);
	bool setVertexWeights(const int vertexIndex, Array* inBoneIDs, Array* inVertexWeights, Tab<INode*> inSkinBones);
};


//class SkinDataRestorObject : public RestoreObj {
//private:
//	static SkinData* skinData;
//public:
//	BOOL updateView;
//
//	SkinDataRestorObject(SkinData* inSkinData, BOOL updateView = TRUE);
//	~SkinDataRestorObject();
//	void Restore(int isUndo);
//	void Redo();
//	void EndHold();
//	TSTR Description();
//};


