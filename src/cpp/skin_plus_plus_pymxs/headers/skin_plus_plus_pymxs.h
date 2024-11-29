#pragma once
#include <skin_plus_plus_py.h>

#include <fmt/xchar.h>

#include <algorithm>
#include <cwchar>
#include <locale>
#include <string.h>

#include <inode.h>
#include <iskin.h>
#include <istdplug.h>
#include <modstack.h>
#include <polyobj.h>
#include <iepoly.h>
#include <Max.h>
#include <matrix3.h>
#include <maxscript/maxwrapper/mxsobjects.h>
//#include <modifiers/bonesdef/bonesdef.h>


class SkinManager
{
private:
	// Whether or not the skin manager is in a valid state.
	// Either no node with the given name can be found, or
	// the given node has no skin modifier on it.
	bool isValid = false;

	// The node with the skin modifier on it:
	INode* node;

	// Skin modifier, used to get the interfaces
	Modifier* skinModifier;

	// Interface for the skin modifier, used to query bone objects when setting skin weights
	ISkin* iSkin;

	// Used to query skin data without selecting skin modifier
	ISkinContextData* iSkinContextData; 

	// Used to modify skin data without selecting skin modifier
	ISkinImportData* iSkinImportData;

	// Used to track the maximum number of vertex weights, allowing data to be resized only when needed
	UINT maximumVertexWeightCount;

	PySkinData* pySkinData;

	// Get the vertex weights and bone ids and add them to the given PySkinData
	void extractWeightsAndBoneIDs(const UINT vertexIndex, const UINT arrayIndex);
	void extractWeightsAndBoneIDs(const UINT vertexIndex);

	// Get the vertex weights, bone ids and positions - on an editable mesh:
	void extractDataMesh(const UINT vertexCount, VertexIDsMatrix vertexIDs);
	void extractDataMesh(const UINT vertexCount);

	// Get the vertex weights, bone ids and positions - on an editable poly:
	void extractDataPoly(const UINT vertexCount, VertexIDsMatrix vertexIDs);
	void extractDataPoly(const UINT vertexCount);
	
	// Add missing bones to the skin modifier based on the given vector of missing bone names
	void addMissingBones(std::vector<std::wstring> missingBoneNames);

	// Initialise skinModifier, iSkin, iSkinContextData, iSkinImportData
	bool initialiseSkin();

public:
	SkinManager(const wchar_t* name) { this->initialise(name); }
	SkinManager(ULONG handle) { this->initialise(handle); }
	~SkinManager(){}

	// Initialise the skin manager with the given node handle
	bool initialise(ULONG handle);

	// Initialise the skin manager with the given node name
	bool initialise(const wchar_t* name);
	
	// Get the vertex weights, bone ids and positions from the given node
	PySkinData* extractSkinData(std::optional<VertexIDsMatrix> vertexIDs = std::nullopt);

	// Set the skin weights to the given node's skin modifier
	bool applySkinData(PySkinData& skinData);
};
