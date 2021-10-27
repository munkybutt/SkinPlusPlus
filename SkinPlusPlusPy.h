#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "SkinPlusPlus.h"


namespace py = pybind11;


class SkinDataPy: SkinData
{
//private:

public:
	SkinDataPy() {};
	py::array_t<double> getSkinWeights();
};

py::array_t<double> getSkinWeights();

