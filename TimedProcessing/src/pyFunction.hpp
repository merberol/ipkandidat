#include "pyhelper.hpp"
#include <iostream>
#include <vector>
#include <utility>

using valueTypePair = std::pair<std::string, std::string>;


class PyFunc {
public:
	CPyInstance pyinstance;
	CPyObject pName;
	CPyObject pModule;
	CPyObject pFunc;
	
	PyFunc(std::string fileName, std::string functionName, std::vector<valueTypePair> argValueTypes)
	{	
		pName = PyUnicode_FromString(fileName.c_str());
		pModule = PyImport_Import(pName);
		if (pModule) { pFunc = PyObject_GetAttrString(pModule, functionName.c_str()); }
		else {
			std::cerr << "error when loading pModule in pyFunction.hpp row 19" << std::endl;
		}
	};

	template <bool, class...T>
	bool call(T...args) {
		// code
	}

private:
int foo() {
	if (pModule) {
		
		if (pFunc && PyCallable_Check(pFunc)) {
			CPyObject arga = PyLong_FromLong(1L);
			CPyObject argb = PyLong_FromLong(3L);
			CPyObject args = PyTuple_New(2);
			PyTuple_SetItem(args, 0, arga);
			PyTuple_SetItem(args, 1, argb);
			CPyObject pRes = PyObject_CallObject(pFunc, args);
			long cRes = PyLong_AsLong(pRes);
			std::cout << "result of pycode = " << cRes << std::endl;
		}
	}
	return 0;
}