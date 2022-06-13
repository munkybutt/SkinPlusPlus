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
#define FMT_DEPRECATED_INCLUDE_XCHAR
#include <fmt/format.h>
//#include <fmt/xchar.h>


namespace py = pybind11;
namespace eg = Eigen;

template <typename T>
int getItemIndex(std::vector<T> vector, T item) {
	auto index = std::distance(vector.begin(), find(vector.begin(), vector.end(), item));
	if (index >= vector.size())
	{
		return NULL;
	}
	return index;
}


struct SortedBoneNameData
{
	std::vector<int> sortedBoneIDs;
	std::vector<std::string> unfoundBoneNames;
	SortedBoneNameData(int boneCount)
	{
		sortedBoneIDs = std::vector<int>(boneCount);
		unfoundBoneNames = std::vector<std::string>();
	};
	~SortedBoneNameData() {}
};


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


	~PySkinData() {}

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

	// Get the bone ids in their correct order as well as any missing bones
	// for the current skin modifier.
	SortedBoneNameData getSortedBoneIDs(std::vector<std::string> currentSkinnedBoneNames)
	{
		auto cachedSize = this->boneNames.size();
		auto size = currentSkinnedBoneNames.size();
		auto sortedBoneNameData = SortedBoneNameData(cachedSize);
		for (size_t boneIndex = 0; boneIndex < cachedSize; boneIndex++)
		{
			std::string boneNameToFind = this->boneNames[boneIndex];
			//auto index = getItemIndex(this->boneNames, boneNameToFind);
			auto index = getItemIndex(currentSkinnedBoneNames, boneNameToFind);
			py::print("boneNameToFind: ", boneNameToFind);
			py::print("index: ", index);
			if (index >= 0)
			{
				sortedBoneNameData.sortedBoneIDs[boneIndex] = index;
				continue;
			}
			sortedBoneNameData.unfoundBoneNames.push_back(boneNameToFind);
			py::print("Bone not found in current skin definition: ", boneNameToFind);
			//auto index = std::distance(skinBoneNames.begin(), find(skinBoneNames.begin(), skinBoneNames.end(), boneNameToFind));
			//if (index >= size)
			//{
			//	sortedBoneNameData.unfoundBoneNames.push_back(boneNameToFind);
			//	continue;
			//}
			//sortedBoneNameData.sortedBoneIDs[boneIndex] = index;
		}
		return sortedBoneNameData;
	}
};





//class PySkinManager
//{
//private:
//	// Whether or not the skin manager is in a valid state.
//	// Either no node with the given name can be found, or
//	// the given node has no skin modifier on it.
//	bool isValid = false;
//
//	// Used to track the maximum number of vertex weights, allowing data to be resized only when needed
//	int maximumVertexWeightCount;
//
//	PySkinData* pySkinData;
//
//	// Get the vertex weights and bone ids and add them to the given PySkinData
//	void collectWeightsAndBoneIDs(unsigned int vertexIndex);
//
//public:
//	PySkinManager(const wchar_t* name) { this->initialise(name); }
//	~PySkinManager() {}
//
//	// Initialise the skin manager with the given node name
//	virtual bool initialise(const wchar_t* name) { return true; };
//
//	// Get the skin weights from the given node's skin modifier
//	std::vector<std::vector<std::vector <float>>> getSkinWeights();
//
//	// Get the vertex weights, bone ids and positions from the given node
//	PySkinData* getData();
//
//	// Set the skin weights to the given node's skin modifier
//	bool setSkinWeights(PySkinData& skinData);
//};