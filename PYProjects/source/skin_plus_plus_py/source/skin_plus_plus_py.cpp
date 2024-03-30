#pragma once
#include <skin_plus_plus_py.h>


PYBIND11_MODULE(skin_plus_plus_py, m) {

	py::class_<PySkinData>(m, "SkinData")
		.def(py::init<>())
		.def(
			py::init<VertexIDsMatrix, int>(),
			py::arg("vertex_ids"),
			py::arg("max_influence_count")
		)
		.def(
			py::init<int, int>(),
			py::arg("vertex_count"),
			py::arg("max_influence_count")
		)
		.def(
			py::init<PySkinData>(),
			py::arg("skin_data")
		)
		.def(
			py::init<
			BoneNamesVector,
			BoneIDsMatrix,
			WeightsMatrix,
			PositionMatrix,
			std::optional<VertexIDsMatrix>
			>(),
			py::arg("skin_bone_names"),
			py::arg("vertex_bone_ids"),
			py::arg("vertex_weights"),
			py::arg("vertex_positions"),
			py::arg("vertex_ids") = py::none()
		)
		.def(py::init<py::tuple>(), py::arg("skin_tuple"))
		.def_readwrite("bone_names", &PySkinData::boneNames)
		.def_readwrite("bone_ids", &PySkinData::boneIDs)
		.def_readwrite("weights", &PySkinData::weights)
		.def_readwrite("positions", &PySkinData::positions)
		.def_readonly("bone_names_copy", &PySkinData::boneNames, py::return_value_policy::copy)
		.def_readonly("bone_ids_copy", &PySkinData::boneIDs, py::return_value_policy::copy)
		.def_readonly("weights_copy", &PySkinData::weights, py::return_value_policy::copy)
		.def_readonly("positions_copy", &PySkinData::positions, py::return_value_policy::copy)
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
