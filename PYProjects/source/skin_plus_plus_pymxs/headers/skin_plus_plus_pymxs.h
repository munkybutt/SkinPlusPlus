#pragma once
#include <skin_plus_plus_py.h>

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

	// Used to query skin data without selecing skin modifier
	ISkinContextData* iSkinContextData; 

	// Used to modify skin data without selecting skin modifier
	ISkinImportData* iSkinImportData;

	// Used to track the maximum number of vertex weights, allowing data to be resized only when needed
	int maximumVertexWeightCount;

	PySkinData* pySkinData;

	// Get the vertex weights and bone ids and add them to the given PySkinData
	void collectWeightsAndBoneIDs(PySkinData* pySkinData, unsigned int vertexIndex);

	// Get the vertex weights, bone ids and positions - on an editable mesh:
	PySkinData* getDataMesh(int vertexCount);

	// Get the vertex weights, bone ids and positions - on an editable poly:
	PySkinData* getDataPoly(int vertexCount);

public:
	SkinManager() {};
	SkinManager(const wchar_t* name) { this->initialise(name); }
	~SkinManager(){}

	// Initialise the skin manager with the given node name
	bool initialise(const wchar_t* name);

	// Get the skin weights from the given node's skin modifier
	std::vector<std::vector<std::vector <float>>> getSkinWeights();

	// Get the vertex weights, bone ids and positions from the given node
	PySkinData* getData();

	// Set the skin weights to the given node's skin modifier
	bool setSkinWeights(PySkinData& skinData);
};
