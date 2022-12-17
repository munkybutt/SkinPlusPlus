#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <Eigen/Geometry>

#include <cwchar>
#include <locale>
#include <string.h>
#include <unordered_map>

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
		return -1;
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
			auto index = getItemIndex(currentSkinnedBoneNames, boneNameToFind);
			if (index != -1)
			{
				sortedBoneNameData.sortedBoneIDs[boneIndex] = index;
				continue;
			}
			sortedBoneNameData.unfoundBoneNames.push_back(boneNameToFind);
			py::print("Bone not found in current skin definition: ", boneNameToFind);
		}
		return sortedBoneNameData;
	}

	//#include <algorithm>
	//#include <unordered_map>
	//#include <vector>
	//SortedBoneNameData getSortedBoneIDs(
	//	const std::vector<std::vector<int>>&indices,
	//	const std::vector<std::string>&old_names,
	//	const std::vector<std::string>&new_names
	//)
	//{

	//	auto cachedSize = this->boneNames.size();
	//	auto sortedBoneNameData = SortedBoneNameData(this->boneNames.size());


	//	std::unordered_map<std::string, int> name_map;
	//	for (size_t i = 0; i < new_names.size(); i++)
	//	{
	//		name_map[new_names[i]] = i;
	//	}

	//	std::vector<std::vector<int>> sorted_bone_ids;
	//	for (const auto& vert_bone_ids : indices)
	//	{
	//		std::vector<int> sorted_ids;
	//		//for (const auto bone_id : vert_bone_ids)
	//		for (size_t bone_id = 0; bone_id < vert_bone_ids.size(); bone_id++)
	//		{
	//			try
	//			{
	//				sortedBoneNameData.sortedBoneIDs[bone_id] = name_map.at(old_names[bone_id]);
	//				//sorted_ids.push_back(name_map.at(old_names[bone_id]));
	//			}
	//			catch (std::out_of_range&)
	//			{
	//				throw std::invalid_argument("Old bone name not found in new names");
	//			}
	//		}
	//		sorted_bone_ids.push_back(sorted_ids);
	//	}
	//	return sortedBoneNameData;
	//}
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