#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <Eigen/Geometry>

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

struct VertexData
{
private:
	Tab<INode*> bones;
	Tab<float> weights;

public:
	VertexData() {};
	VertexData(int vertexID, py::array_t<float> boneIDs, py::array_t<float> weights, Tab<INode*> skinBones);
	void initialiseVariables(int size);
	void appendVariables(INode* bone, float weight);
	Tab<INode*> getBones() { return this->bones; };
	Tab<float> getWeights() { return this->weights; };
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

public:
	SkinData() {};
	SkinData(const wchar_t* name) { this->initialise(name); }
	~SkinData(){}
	bool initialise(const wchar_t* name);
	std::vector<std::vector<std::vector <float>>> getSkinWeights();
	//bool setSkinWeights(Array* boneIDs, Array* vertexWeights, Array* vertices = NULL);
	//bool setSkinWeights(py::array_t<py::array_t<float>> boneIDs, py::array_t<py::array_t<float>> vertexWeights);
	//bool setSkinWeights(py::array_t<float> boneIDs, py::array_t<float> vertexWeights);
	bool setSkinWeights(Eigen::MatrixXf& boneIDs, Eigen::MatrixXf& vertexWeights);
	bool setVertexWeights(int vertexIndex, Array* vertexBones, Array* vertexWeights);
	bool setVertexWeights(const int vertexIndex, Array* inBoneIDs, Array* inVertexWeights, Tab<INode*> inSkinBones);
};