#ifndef PYHELPER_HPP
#define PYHELPER_HPP
#pragma once

#include "Python.h"

class CPyInstance
{
public:
	CPyInstance()
	{
		/*{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "Initializing Python\n";
			outfile.close();
		}*/
		Py_Initialize();
	}

	~CPyInstance()
	{
		/*{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "Deinitializing Python\n";
			outfile.close();
		}*/
		Py_Finalize();
	}
};


class CPyObject
{
private:
	PyObject *p;
public:
	CPyObject() : p(NULL)
	{
		/*{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "Creating empty CPyObject\n";
			outfile.close();
		}*/
	}

	CPyObject(PyObject* _p) : p(_p)
	{
		/*{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "CPyObject Copy constructor\n";
			outfile.close();
		}*/
	}

	
	~CPyObject()
	{
		/*{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "CPyObject Destructor\n";
			outfile.close();
		}*/
		Release();
	}

	PyObject* getObject()
	{
		return p;
	}

	PyObject* setObject(PyObject* _p)
	{
		return (p=_p);
	}

	PyObject* AddRef()
	{
		/*{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "CPyObject add ref\n";
			outfile.close();
		}*/
		if(p)
		{
			Py_INCREF(p);
		}
		return p;
	}

	void Release()
	{
		/*{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "CPyObject remove ref\n";
			outfile.close();
		}*/
		if(p)
		{
			Py_DECREF(p);
		}

		p= NULL;
	}

	PyObject* operator ->()
	{
		return p;
	}

	bool is()
	{
		return p ? true : false;
	}

	operator PyObject*()
	{
		return p;
	}

	PyObject* operator = (PyObject* pp)
	{
		p = pp;
		return p;
	}

	operator bool()
	{
		return p ? true : false;
	}
};


#endif