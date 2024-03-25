#pragma once
#include <skin_plus_plus_py.h>


PYBIND11_MODULE(skin_plus_plus_py, m) {

	py::class_<PySkinData>(m, "SkinData")
		.def(py::init<int, int>())
		.def(py::init<BoneNamesVector, BoneIDsMatrix, WeightsMatrix, PositionMatrix>())
		.def(py::init<py::tuple>())
		.def_readwrite("bone_names", &PySkinData::boneNames)
		.def_readwrite("bone_ids", &PySkinData::boneIDs)
		.def_readwrite("weights", &PySkinData::weights)
		.def_readwrite("positions", &PySkinData::positions)
		.def(
			py::pickle(
				[](const PySkinData& pySkinData) { // __getstate__
					return py::make_tuple(
						pySkinData.boneNames,
						pySkinData.boneIDs,
						pySkinData.weights,
						pySkinData.positions
					);
				},
				[](py::tuple data) { // __setstate__

					PySkinData pySkinData(data);

					return pySkinData;
				}
			)
		)
		.def(
			"__repr__", [](const PySkinData& o) {
				auto vertexCount = o.boneIDs.rows();
				auto influenceCount = o.boneIDs.cols();
				return fmt::format("<SkinData({}x{})>", vertexCount, influenceCount);
			}
		);
};
