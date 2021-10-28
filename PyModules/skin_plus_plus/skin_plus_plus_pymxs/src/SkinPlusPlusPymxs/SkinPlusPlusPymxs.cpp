#pragma once
#include <SkinPlusPlusPymxs.h>


char convertWCharToChar(const wchar_t* text)
{
	size_t length = std::wcslen(text);
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> conv;
	std::string StoreTextBuffer = conv.to_bytes(text, text + length);

	return StoreTextBuffer[0];
}


INode* getChildByName(const wchar_t* name, INode* parent)
{
	INode* parent_ = parent;
	if (!parent)
	{
		Interface* ip = GetCOREInterface();
		parent_ = ip->GetRootNode();
	}
	INode* node;
	const wchar_t* nodeName;
	for (int i = 0; i < parent_->NumberOfChildren(); i++)
	{
		node = parent_->GetChildNode(i);
		nodeName = node->GetName();
		if (wcscmp(nodeName, name) == 0) return node;
		try { node = getChildByName(name, parent = node); } catch (const std::invalid_argument&) { continue; }
		return node;
	}
	//name->length();
	throw std::invalid_argument("No node with name: " + convertWCharToChar(name));
}


bool SkinData::initialise(const wchar_t* name)
{
	this->node = getChildByName(name, NULL);
	if (!this->node)
	{
		throw pybind11::type_error("SkinData init failed. No node with name: " + convertWCharToChar(name));
	}
	Object* object = this->node->GetObjectRef();
	if (!object || (object->SuperClassID() != GEN_DERIVOB_CLASS_ID))
	{
		throw pybind11::type_error("SkinData init failed. Node is incorrect type " + convertWCharToChar(this->node->GetName()));
	}
	this->iDerivedObject = (IDerivedObject*)(object);
	if (!this->iDerivedObject)
	{
		this->isValid = false;
		return this->isValid;
	}

	for (int modifierIndex = 0; modifierIndex < this->iDerivedObject->NumModifiers(); modifierIndex++)
	{
		this->skinModifier = this->iDerivedObject->GetModifier(modifierIndex);
		if (this->skinModifier->ClassID() != SKIN_CLASSID) continue;

		this->iSkin = (ISkin*)this->skinModifier->GetInterface(I_SKIN);
		if (!this->iSkin) continue;

		this->iSkinContextData = this->iSkin->GetContextInterface(this->node);
		if (!this->iSkinContextData) continue;

		this->iSkinImportData = (ISkinImportData*)this->skinModifier->GetInterface(I_SKINIMPORTDATA);
		if (!this->iSkinImportData) continue;

		this->isValid = true;
		return this->isValid;
	}
	throw std::exception("SkinData init failed on node: " + convertWCharToChar(name));
}


std::vector<std::vector<std::vector <float>>> SkinData::getSkinWeights()
{
	if (!this->isValid)
	{
		throw std::exception("SkinData object is invalid. Cannot get skin weights.");
	}
	unsigned int vertexCount = this->iSkinContextData->GetNumPoints();
	std::vector<std::vector<std::vector <float>>> skinDataArray(
		2, std::vector<std::vector<float>>(vertexCount)
	);
	//SkinArray skinDataArray(2, VertexArray(vertexCount));
	for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		auto influenceCount = this->iSkinContextData->GetNumAssignedBones(vertexIndex);
		std::vector<float> influenceWeights(influenceCount);
		std::vector<float> influenceBoneIDs(influenceCount);
		//influenceWeights[0] = 1.0f;
		//influenceWeights->data()[0] = 1.0f;
		skinDataArray[0][vertexIndex] = influenceWeights;  //influenceWeights
		skinDataArray[1][vertexIndex] = influenceBoneIDs;  //influenceBoneIDs
		for (auto influenceIndex = 0; influenceIndex < influenceCount; influenceIndex++)
		{
			auto infuenceWeight = this->iSkinContextData->GetBoneWeight(vertexIndex, influenceIndex);
			if (infuenceWeight <= 0.0f) continue;
			auto influenceBoneID = this->iSkinContextData->GetAssignedBone(vertexIndex, influenceIndex);
			influenceWeights[influenceIndex] = infuenceWeight;
			influenceBoneIDs[influenceIndex] = float(influenceBoneID);
		}
	};
	return skinDataArray;
}

auto SkinData::getSkinWeightsPy(const int flag)
{
	auto weights = this->getSkinWeights();
	switch (flag) {
		case 0:
		{
			py::array npArray = py::cast(weights);
			return npArray;
		}
		case 1:
		{
			py::array npArray = py::cast(weights);
			return npArray;
		}
		default:
		{
			py::array npArray = py::cast(weights);
			return npArray;
		}
	}
}


inline std::vector<std::vector<std::vector <float>>> getSkinWeights(wchar_t* name)
{
	SkinData* skinData = new SkinData(name);
	return skinData->getSkinWeights();
}


template <typename Sequence>
inline py::array_t<typename Sequence::value_type> as_pyarray(Sequence&& seq) {
	auto size = seq.size();
	auto data = seq.data();
	std::unique_ptr<Sequence> seq_ptr = std::make_unique<Sequence>(std::move(seq));
	auto capsule = py::capsule(seq_ptr.get(), [](void* p) { std::unique_ptr<Sequence>(reinterpret_cast<Sequence*>(p)); });
	seq_ptr.release();
	return py::array(size, data, capsule);
}


template <typename Sequence>
inline py::array_t<py::array_t<py::array_t<float>>> as_pyfloatarray(Sequence&& seq) {
	auto size = seq.size();
	auto data = seq.data();
	std::unique_ptr<Sequence> seq_ptr = std::make_unique<Sequence>(std::move(seq));
	auto capsule = py::capsule(seq_ptr.get(), [](void* p) { std::unique_ptr<Sequence>(reinterpret_cast<Sequence*>(p)); });
	seq_ptr.release();
	return py::array(size, data, capsule);
}

//template <typename Sequence>
//inline py::array_t<typename Sequence::value_type> asNestedPyArray(Sequence&& seq) {
//
//}




PYBIND11_MODULE(SkinPlusPlusPymxs, m) {

	py::class_<SkinData>(m, "SkinData")
		.def(py::init<>())
		.def(py::init<const wchar_t*>())
		.def("initialise", &SkinData::initialise)
		.def("get_skin_weights", &SkinData::getSkinWeightsPy)
	;
	//def("__init__", [](...) { ... }, py::arg().noconvert(), py::arg("arg2") = false);
	m.def("get_skin_weights", [](wchar_t* name, int return_type) {
			std::vector<std::vector<std::vector <float>>> weights = getSkinWeights(name);
			switch (return_type) {
				case 0:
					return py::cast(weights);
				case 1:
					return py::cast(weights, py::return_value_policy::take_ownership);
				case 2:
					return py::cast(weights, py::return_value_policy::copy);
				case 3:
					return py::cast(weights, py::return_value_policy::move);
				case 4:
					return py::cast(weights, py::return_value_policy::reference);
				case 5:
					return py::cast(weights, py::return_value_policy::reference_internal);
				case 6:
					return py::cast(weights, py::return_value_policy::automatic);
				case 7:
					return py::cast(weights, py::return_value_policy::automatic_reference);
				default:
					return py::cast(weights);
			}
			
		}, "Get Skin Weights",
		py::arg("name"),
		py::arg("return_type")
	);
	//m.def("f", []() {
	//		// Allocate and initialize some data; make this big so
	//		// we can see the impact on the process memory use:
	//		constexpr size_t size = 100 * 1000 * 1000;
	//		double* foo = new double[size];
	//		for (size_t i = 0; i < size; i++) {
	//			foo[i] = (double)i;
	//		}

	//		// Create a Python object that will free the allocated
	//		// memory when destroyed:
	//		py::capsule free_when_done(foo, [](void* f) {
	//			double* foo = reinterpret_cast<double*>(f);
	//			std::cerr << "Element [0] = " << foo[0] << "\n";
	//			std::cerr << "freeing memory @ " << f << "\n";
	//			delete[] foo;
	//			});

	//		return py::array_t<double>(
	//			{ 100, 1000, 1000 }, // shape
	//			{ 1000 * 1000 * 8, 1000 * 8, 8 }, // C-style contiguous strides for double
	//			foo, // the data pointer
	//			free_when_done // numpy array references this parent
	//		);
	//	}
	//);
	//m.def("f", [&](wchar_t* name) {
	//		std::vector<std::vector<std::vector <float>>> weights = getSkinWeights(name);
	//		py::print(weights[0].size());
	//		py::array_t<py::array_t<float>>({ 2, weights[0].size() }, weights[0])
	//		
	//		return py::array_t<py::array_t<py::array_t<float>>>(
	//			{ 2, weights[0].size() }, // shape
	//			{ 1000 * 1000 * 8, 1000 * 8, 8 }, // C-style contiguous strides for double
	//			weights // the data pointer
	//		);
	//	}
	//);
}