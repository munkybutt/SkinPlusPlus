#pragma once

#include "SkinPlusPlusPy.h"


//py::array_t<py::array_t<double>> SkinDataPy::getSkinWeights()
py::array_t<double> SkinDataPy::getSkinWeights()
{
	//int vertexCount = this->iSkinContextData->GetNumPoints();
	//weights = new Array(vertexCount);
	//boneIDs = new Array(vertexCount);
	//weights->size = vertexCount;
	//boneIDs->size = vertexCount;
	//for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	//{
	//	int influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
	//	vl.influenceWeights = new Array(influenceCount);
	//	vl.influenceBoneIDs = new Array(influenceCount);
	//	vl.influenceWeights->size = influenceCount;
	//	vl.influenceBoneIDs->size = influenceCount;
	//	for (int influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
	//	{
	//		float infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
	//		int influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex) + 1;
	//		vl.influenceWeights->data[influenceIndex] = Float::intern(infuenceWeight);
	//		vl.influenceBoneIDs->data[influenceIndex] = Integer::intern(influenceBoneID);
	//	}
	//	vl.weights->data[vertexIndex] = vl.influenceWeights;
	//	vl.boneIDs->data[vertexIndex] = vl.influenceBoneIDs;
	//};
	//vl.skinData = new Array(2);
	//vl.skinData->size = 2;
	//vl.skinData->data[0] = vl.weights;
	//vl.skinData->data[1] = vl.boneIDs;
	//return_value_tls(vl.weights);

	py::array_t<double> sub_array = py::array_t<double>(10);
	//py::array_t<py::array_t<double>> array = py::array_t<py::array_t<double>>(1);
	//array[0] = sub_array;
	return sub_array;
}



py::array_t<double> getSkinWeights()
{
	SkinDataPy* skinDataPy = new SkinDataPy();
	return skinDataPy->getSkinWeights();
};



PYBIND11_MODULE(SkinPlusPlusPy, m) {
	m.def("get_skin_weights", &getSkinWeights, "Get Skin Weights");

//#ifdef VERSION_INFO
//    m.attr("__version__") = VERSION_INFO;
//#else
//    m.attr("__version__") = "dev";
//#endif
}