#pragma once

#include <iostream>
#include <string>
#include <direct.h>
#include <vector>
#include "includes/Python39/pyhelper.hpp"

#ifndef DEBUG
#define DEBUG
#endif
class PyFunc {
	
	//std::vector<RefTypePair> argValueTypes;
	CPyInstance instance;

	CPyObject pModule;
	
	std::string fileName;
	
	int argc;
public:	

	PyFunc(std::string _fileName, int _argc)
	:fileName{_fileName}, argc{_argc}
	{
	#ifdef DEBUG
		std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\nflight loop\n*************\n";
		outfile.close();
	#endif
	};

	bool call(std::vector<CPyObject> _args) {
	#ifdef DEBUG
		std::ofstream outfile;

 		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\nIn Call\n*************\n";
		outfile << " calling : " << fileName << "\n";
		outfile << " length of args vec = " << _args.size() << "\n";
		outfile << " expected length by argc = " << argc << "\n";
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
		pModule = PyImport_Import(pName);

		if (pModule) {
			pFunc = PyObject_GetAttrString(pModule, "main");
	
		}
		else {
			std::cerr << "error when loading pModule in pyFunction.hpp" << std::endl;
		}

		bool res{};
		CPyObject pRes;

		if (argc > 0) {
			if ( pFunc && PyCallable_Check(pFunc))
			{
				CPyObject args = PyTuple_New(argc);
				std::cout << "created tuple " << std::endl;
	#ifdef DEBUG
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << " before loop \n";
				outfile.close();
	#endif		
			
				for (int i = 0 ; i < argc ; i++) {

					PyTuple_SetItem(args, i, _args[i]);
				}


	#ifdef DEBUG
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << " after loop \n";
				outfile.close();
	#endif
				
				try{
					
					std::cout << "after assert  before python call" << std::endl;
					pRes = PyObject_CallObject(pFunc, args);
					std::cout << "result of python call: " <<  PyLong_AsLong(pRes) << std::endl;
				}catch ( std::exception & e) {
					std::cout << e.what() << std::endl;
				}
				res = PyLong_AsLong(pRes) >= 1;
			}
		}
		else {
			if ( pFunc && PyCallable_Check(pFunc))
			{
				std::cout << "argc is 0" << std::endl;

				pRes = PyObject_CallObject(pFunc, NULL);
				std::cout << "running pFunc" << std::endl;
				res = PyLong_AsLong(pRes) >= 1;
				std::cout << "converting pRes" << std::endl;
			}
		}
		std::cout << "returnning " << std::endl;
	#ifdef DEBUG
		outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile << " Exiting Call \n";
			outfile.close();
	#endif
		return res;
	}


};