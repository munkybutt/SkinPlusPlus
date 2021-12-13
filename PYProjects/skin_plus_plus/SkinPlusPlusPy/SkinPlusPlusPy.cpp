#include <SkinPlusPlus.h>


PYBIND11_MODULE(SkinPlusPlusPy, m) {

	py::class_<PySkinData>(m, "SkinData")
		.def(py::init<int, int>())
		.def_readonly("bone_ids", &PySkinData::boneIDs)
		.def_readonly("weights", &PySkinData::weights)
		.def_readonly("positions", &PySkinData::positions)
		.def(
			py::pickle(
				[](const PySkinData& pySkinData) { // __getstate__
					return py::make_tuple(pySkinData.boneIDs, pySkinData.weights, pySkinData.positions);
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