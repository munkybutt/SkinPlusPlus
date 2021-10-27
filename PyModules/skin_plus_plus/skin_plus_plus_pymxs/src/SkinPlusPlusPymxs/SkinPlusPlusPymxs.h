#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <cwchar>
#include <locale>
#include <string.h>

#include <inode.h>
#include <iskin.h>
#include <istdplug.h>
#include <maxscript/util/listener.h>
#include <modstack.h>


namespace py = pybind11;


//typedef std::vector<std::vector<std::vector <float>>> SkinDataVector_T;

//using ValueArray = std::vector <float>;
//using VertexArray = std::vector <ValueArray>;
//using SkinArray = std::vector <VertexArray>;


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

public:
	SkinData() {};
	SkinData(const wchar_t* name) { this->initialise(name); }
	~SkinData(){}
	bool initialise(const wchar_t* name);
	std::vector<std::vector<std::vector <float>>> getSkinWeights();
	auto getSkinWeightsPy(const int flag);
	auto getSkinWeights2();
	bool setSkinWeights(Array* boneIDs, Array* vertexWeights, Array* vertices = NULL);
	bool setVertexWeights(int vertexIndex, Array* vertexBones, Array* vertexWeights);
	bool setVertexWeights(const int vertexIndex, Array* inBoneIDs, Array* inVertexWeights, Tab<INode*> inSkinBones);
};