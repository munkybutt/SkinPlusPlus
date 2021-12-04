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
#include <polyobj.h>
#include <iepoly.h>
#include <Max.h>
#include <istdplug.h>
#include <matrix3.h>

#define FMT_HEADER_ONLY
//#include <fmt/core.h>
#include <fmt/format.h>
//#include <fmt/format-inl.h>
//#include <fmt/format.cc>
//#include "iparamb2.h"
//#include "iparamm2.h"
//#include "notify.h"
//#include "modstack.h"
//#include "macrorec.h"
//#include "utilapi.h"
//#include "iFnPub.h"


namespace py = pybind11;
namespace eg = Eigen;


struct PySkinData final
{
public:
	eg::MatrixXi boneIDs;
	eg::MatrixXf weights;
	eg::MatrixXf positions;

	PySkinData() {}

	PySkinData(int vertexCount, int maxInfluenceCount)
	{
		this->boneIDs = eg::MatrixXi(vertexCount, maxInfluenceCount);
		this->weights = eg::MatrixXf(vertexCount, maxInfluenceCount);
		this->positions = eg::MatrixXf(vertexCount, 3);
	}

	PySkinData(py::tuple data)
	{
		if (data.size() != 3)
			throw std::runtime_error("Invalid state!");

		this->setInternalState(data);
	}

	// Set the internal state of the object, replacing all data.
	// The tuple structure is: (boneIDs, weights, positions).
	void setInternalState(py::tuple data);

	// Set a new maximum influence count
	void setMaximumInfluenceCount(int influenceCount)
	{
		this->boneIDs.resize(eg::NoChange, influenceCount);
		this->weights.resize(eg::NoChange, influenceCount);
	}

	~PySkinData() {}

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
	int maxInfluenceCount;

private:
	void collectWeightsAndBoneIDs(PySkinData* pySkinData, unsigned int vertexIndex);

public:
	SkinData() {};
	SkinData(const wchar_t* name) { this->initialise(name); }
	~SkinData(){}
	bool initialise(const wchar_t* name);
	std::vector<std::vector<std::vector <float>>> getSkinWeights();
	PySkinData* getDataMesh();
	PySkinData* getDataPoly();
	PySkinData* getData();
	bool setSkinWeights(eg::MatrixXi& boneIDs, eg::MatrixXf& vertexWeights);
};