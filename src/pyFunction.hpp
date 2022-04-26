#pragma once

#include <iostream>
#include <string>
#include <direct.h>
#include <vector>
#include "includes/Python39/pyhelper.hpp"
#include "pyVar.hpp"


#define DEBUG

class PyFunc {
	/*
	CPyInstance instance;
	std::string fileName;
	int argc;
public:	

	PyFunc(std::string _fileName, int _argc)
	:fileName{_fileName}, argc{_argc}
	{
		std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************creating Pyfunc*************\n";
		outfile.close();
	};

	bool call(std::vector<PyVar> & _args) {
	#ifdef DEBUG
		std::ofstream outfile;

 		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n*************** In Func Call *************\n";
		outfile << " calling : " << this->fileName << "\n";
		outfile << " length of args vec = " << _args.size() << "\n";
		outfile << " expected length by argc = " << this->argc << "\n";
		outfile.close();
	#endif
		CPyObject pFunc;
		std::cout << "call" << std::endl;
		assert( _args.size() == argc);
		std::cout << "after assert  2" << std::endl;
		CPyObject pName = PyUnicode_FromString( fileName.c_str());

		if (!(pName)){
			std::cerr << "error when loading pName in pyFunction.hpp" << std::endl;
			return false;
		}
		CPyObject pModule = PyImport_Import(pName);

		if (pModule) {
			pFunc = PyObject_GetAttrString(pModule, "main");
	
		}
		else {
#ifdef DEBUG
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << " Error when loading file \n";
				outfile.close();
				exit(1);
#endif	
		}

		bool res{};
		CPyObject pRes;
		CPyObject args;

		if (argc > 0) {
			if ( pFunc && PyCallable_Check(pFunc))
			{
				args = PyTuple_New(argc);
#ifdef DEBUG
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "created tuple ";
				outfile << " before loop \n";
				outfile.close();
#endif		
			
				for (int i = 0 ; i < argc ; i++) {
#ifdef DEBUG
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "setting tuple value " << i << " to " << _args[i].get()<< "\n";
				outfile.close();
#endif	
					PyTuple_SetItem(args, i, _args[i].get());
				}


#ifdef DEBUG
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << " after loop \n";
				outfile.close();
#endif
				
				try {
#ifdef DEBUG
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "after assert before python call\n";
					outfile << "args are " << args << "\n";
					outfile.close();
#endif
					pRes = PyObject_CallObject(pFunc, args);
#ifdef DEBUG	
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "result of python call: " << PyLong_AsLong(pRes) << "\n";
					outfile.close();
#endif
				}
				catch ( std::exception & e) {
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << e.what() << "\n";
					outfile.close();
				}
				res = PyLong_AsLong(pRes);
			}
		}
		else {
			if ( pFunc && PyCallable_Check(pFunc))
			{
#ifdef DEBUG
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "running pFunc with 0 args\n";
				outfile.close();
#endif
				pRes = PyObject_CallObject(pFunc, NULL);
				res = PyLong_AsLong(pRes);
#ifdef DEBUG
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "converting pRes to value " << res <<  "\n";
				outfile.close();
#endif
			}
		}
#ifdef DEBUG
		outfile.open("liuHapticLog.txt", std::ios_base::app);
		outfile << "returning from pyFunction\n";
		outfile << "Exiting Call \n";
		outfile.close();
#endif
		return res;
	}
*/

};