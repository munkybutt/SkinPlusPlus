#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <Eigen/Geometry>

#include <cwchar>
#include <locale>
#include <string.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>


namespace py = pybind11;
namespace eg = Eigen;


struct PySkinData final
{
public:
	eg::MatrixXi boneIDs;
	eg::MatrixXf weights;
	eg::MatrixXf positions;
	std::vector<std::string> boneNames;

	PySkinData() {}

	PySkinData(int vertexCount, int maxInfluenceCount)
	{
		this->boneIDs = eg::MatrixXi(vertexCount, maxInfluenceCount);
		this->weights = eg::MatrixXf(vertexCount, maxInfluenceCount);
		this->positions = eg::MatrixXf(vertexCount, 3);
	}

	PySkinData(py::tuple data)
	{
		if (data.size() != 4)
			throw std::runtime_error("Invalid state!");

		this->setInternalState(data);
	}

	// Set the internal state of the object, replacing all data.
	// The tuple structure is: (boneNames, boneIDs, weights, positions).
	void setInternalState(py::tuple data)
	{
		this->boneNames = py::cast<std::vector<std::string>>(data[0]);
		this->boneIDs = py::cast<eg::MatrixXi>(data[1]);
		this->weights = py::cast<eg::MatrixXf>(data[2]);
		this->positions = py::cast<eg::MatrixXf>(data[3]);
	}

	// Set a new maximum influence count
	void setMaximumVertexWeightCount(int influenceCount)
	{
		this->boneIDs.resize(eg::NoChange, influenceCount);
		this->weights.resize(eg::NoChange, influenceCount);
	}

	~PySkinData() {}

};
