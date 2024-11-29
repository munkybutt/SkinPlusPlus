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
#include <fmt/xchar.h>

namespace py = pybind11;
namespace eg = Eigen;


typedef std::vector<std::wstring> BoneNamesVector;
typedef eg::MatrixXi BoneIDsMatrix;
typedef eg::MatrixXd WeightsMatrix;
typedef eg::MatrixXd PositionMatrix;
typedef eg::Matrix<int, 1, -1> VertexIDsMatrix;
typedef unsigned int UINT;


template <typename T>
UINT getItemIndex(std::vector<T>& vector, T item) {
    UINT index = std::distance(vector.begin(), find(vector.begin(), vector.end(), item));
    if (index >= vector.size())
    {
        return UINT(-1);
    }
    return index;
}


char convertWCharToChar(const wchar_t* text)
{
    std::wstring ws(text);
    std::string str(ws.begin(), ws.end());

    return str[0];
}


std::string convertWCharToString(const wchar_t* text)
{
    std::wstring ws(text);
    std::string str(ws.begin(), ws.end());

    return str;
}


std::wstring convertStringToWString(const std::string& multi) {
    std::wstring wide;
    wchar_t w;
    mbstate_t mb{};
    size_t n = 0, len = multi.length() + 1;
    while (auto res = mbrtowc(&w, multi.c_str() + n, len - n, &mb)) {
        if (res == size_t(-1) || res == size_t(-2))
            throw "invalid encoding";

        n += res;
        wide += w;
    }
    return wide;
}


const char* convertStringToChar(std::string text)
{
    const char* charArray;
    charArray = &text[0];
    return charArray;
}


struct SortedBoneNameData
{
    std::vector<UINT> sortedBoneIDs;
    std::vector<std::wstring> unfoundBoneNames;
    UINT highestBoneID;
    UINT lowestBoneID;
    SortedBoneNameData(UINT boneCount)
    {
        sortedBoneIDs = std::vector<UINT>(boneCount);
        unfoundBoneNames = std::vector<std::wstring>();
    };
    ~SortedBoneNameData() {}
};


struct PySkinData final
{
public:
    BoneNamesVector boneNames;
    BoneIDsMatrix boneIDs;
    WeightsMatrix weights;
    PositionMatrix positions;
    std::optional<VertexIDsMatrix> vertexIDs = std::nullopt;

    PySkinData() {}

    PySkinData(VertexIDsMatrix vertexIDs, UINT maxInfluenceCount) {
        const eg::Index vertexCount = vertexIDs.size();
        this->boneIDs = BoneIDsMatrix::Constant(vertexCount, maxInfluenceCount, -1);
        this->weights = WeightsMatrix::Zero(vertexCount, maxInfluenceCount);
        this->positions = PositionMatrix(vertexCount, 3);
        this->vertexIDs = vertexIDs;
    }

    PySkinData(UINT vertexCount, UINT maxInfluenceCount, std::optional<VertexIDsMatrix> vertexIDs = std::nullopt)
    {
        this->boneIDs = BoneIDsMatrix::Constant(vertexCount, maxInfluenceCount, -1);
        this->weights = WeightsMatrix::Zero(vertexCount, maxInfluenceCount);
        this->positions = PositionMatrix(vertexCount, 3);
        if (vertexIDs.has_value())
        {
            this->vertexIDs = vertexIDs.value();
        }
    }

    PySkinData(BoneNamesVector boneNames, BoneIDsMatrix boneIDs, WeightsMatrix weights, PositionMatrix positions, std::optional<VertexIDsMatrix> vertexIDs = std::nullopt)
    {
        this->setInternalState(boneNames, boneIDs, weights, positions, vertexIDs);
    }

    PySkinData(py::tuple data)
    {
        this->setInternalState(data);
    }

    PySkinData(const PySkinData& skinData)
    {
        if (skinData.vertexIDs.has_value())
        {
            this->setInternalState(
                skinData.boneNames,
                BoneIDsMatrix(skinData.boneIDs),
                WeightsMatrix(skinData.weights),
                PositionMatrix(skinData.positions),
                VertexIDsMatrix(skinData.vertexIDs.value())
            );
        }
        else
        {
            this->setInternalState(
                skinData.boneNames,
                BoneIDsMatrix(skinData.boneIDs),
                WeightsMatrix(skinData.weights),
                PositionMatrix(skinData.positions)
            );
        }
    }

    ~PySkinData() {}


    // Set the internal state of the object, replacing all data.
    void setInternalState(BoneNamesVector boneNames, BoneIDsMatrix boneIDs, WeightsMatrix weights, PositionMatrix positions, std::optional<VertexIDsMatrix> vertexIDs = std::nullopt)
    {
        this->boneNames = boneNames;
        this->boneIDs = boneIDs;
        this->weights = weights;
        this->positions = positions;
        if (vertexIDs.has_value())
        {
            this->vertexIDs = vertexIDs.value();
        }
    }

    // Set the internal state of the object, replacing all data.
    // The tuple structure is: (boneNames, boneIDs, weights, positions, vertexIDs).
    void setInternalState(py::tuple data)
    {
        auto dataSize = data.size();
        if (dataSize == 5 && !py::isinstance<py::none>(data[4]))
        {
            this->setInternalState(
                py::cast<BoneNamesVector>(data[0]),
                py::cast<BoneIDsMatrix>(data[1]),
                py::cast<WeightsMatrix>(data[2]),
                py::cast<PositionMatrix>(data[3]),
                py::cast<VertexIDsMatrix>(data[4])
            );
        }
        else if (dataSize >= 4)
        {
            this->setInternalState(
                py::cast<BoneNamesVector>(data[0]),
                py::cast<BoneIDsMatrix>(data[1]),
                py::cast<WeightsMatrix>(data[2]),
                py::cast<PositionMatrix>(data[3])
            );
        }
        else
        {
            throw std::runtime_error(
                "Invalid state - The tuple structure is: (bone_names, bone_ids, weights, positions, vertexIDs = (optional)"
            );
        }
    }

    // Set a new maximum influence count
    void setMaximumVertexWeightCount(int influenceCount)
    {
        this->boneIDs.conservativeResize(eg::NoChange, influenceCount);
        this->weights.conservativeResize(eg::NoChange, influenceCount);
    }

    // Get the bone ids in their correct order as well as any missing bones
    // for the current skin modifier.
    SortedBoneNameData getSortedBoneIDs(BoneNamesVector& currentBoneNames)
	{
        const size_t cachedBoneCount = this->boneNames.size();
		auto sortedBoneNameData = SortedBoneNameData(cachedBoneCount);
        std::unordered_map<std::wstring, size_t> nameMap;
        for (size_t index = 0; index < currentBoneNames.size(); index++)
        {
            nameMap[currentBoneNames[index]] = index;
        }

		for (size_t boneIndex = 0; boneIndex < cachedBoneCount; boneIndex++)
		{
			const std::wstring nameToFind = this->boneNames[boneIndex];
            const auto lookup = nameMap.find(nameToFind);
            if (lookup != nameMap.end())
            {
                sortedBoneNameData.sortedBoneIDs[boneIndex] = lookup->second;
            }
            else
            {
                sortedBoneNameData.unfoundBoneNames.push_back(nameToFind);
            }
		}
        if (!sortedBoneNameData.unfoundBoneNames.empty())
        {
            std::wstring message = L"The following bones are not in the skin definition:";
            for (const std::wstring& name : sortedBoneNameData.unfoundBoneNames)
            {
                message = fmt::format(L"{}\n- {}", message, name);
            }
            py::print(message);

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
