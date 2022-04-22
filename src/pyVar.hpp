#pragma once

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <direct.h>
#include <vector>
#include "includes/Python39/pyhelper.hpp"

#ifndef DEBUG
#define DEBUG
#endif
class PyVar{
	

	CPyInstance instance;

	CPyObject var;
	
public:	

	PyVar(int _var)
	{
#ifdef DEBUG
		std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n*************** creating py var *************\n";
		outfile.close();
	#endif
    var = PyLong_FromLong(_var);
	};

    PyVar(double _var)
	{
	#ifdef DEBUG
		std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n*************** creating py var *************\n";
		outfile.close();
	#endif
    var = PyFloat_FromDouble(_var);
	};

	CPyObject get() {
#ifdef DEBUG
		std::ofstream outfile;
 		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\nIn get\n*************\n";
		outfile.close();
#endif
		return var;
	}


};