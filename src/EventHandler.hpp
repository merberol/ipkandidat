#pragma once


#include <windows.h>
#include <unordered_map>
#include <variant>
#include <tuple>
#include <chrono>
#include <iostream> 
#include <stdio.h>

#include <memory>
#include <fstream>
#include "types.h"

#include "ConfigLoader.hpp"
#include "includes\bHaptics\HapticLibrary.h"
#include "XPLMDataAccess.h"
#include "pyVar.hpp"

#define DEBUG

class EventHandler;

class HapticInterface {
	EventToFileVec eventFileVec{};
public:
	HapticInterface();
	~HapticInterface();
	void send(std::string eventName, EventHandler const& eventHandler);
	void addFileMap(EventToFileVec eventFileVec);
};

class EventHandler
{

public:
	std::string _id;
	EventNameMap eventNameMap{};
	EventUsedVec eventUsed{};

	std::vector<std::vector<RefTypePair>> eventTypeRefs{};
	RefPathVector refPathVec{};
	PyFileNameVec pyFuncs{};
	
	EventHandler() = default;

	EventHandler(std::string id) 
		:_id{ id }
	{
		char playerPath[200];
		int esize = 2;
		bool res = TryGetExePath(playerPath, esize);
		auto path = GetExePath();

		if (res) {
			std::cout << "1. getExePath bHaptics Player is installed:  " << playerPath << std::endl << path << std::endl;
			std::cout << "1. player path size:  " << esize << std::endl;
		}
		else {
			std::cout << "1. Cannot find exe path.  " << std::endl;
		}
		std::cout << "Starting initialisation" << std::endl;
		auto start = std::chrono::system_clock::now();
		Initialise(id.c_str(), "SampleApp");
		auto end = std::chrono::system_clock::now();

		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << "2. Initialise() elapsed time: " << elapsed_seconds.count() << "s\n";

		// worker.SayHello();
		ConfigLoader configLoader{};
		configLoader.SayHello();
		TactFileVec tactFiles{};

		configLoader.run(eventNameMap, eventUsed, tactFiles, refPathVec, eventTypeRefs, pyFuncs);

		worker.addFileMap(tactFiles);
	};

	~EventHandler() {
		Destroy();
	}

	void RunEvent(std::string eventName, std::unordered_map<std::string, double> const& dataMap) {
		{
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "\n*************** in EventHandler Run event *************\n";
			outfile.close();
		}
		// first we need to compile arguments
		
		bool result{};
		try{


			int index = getIndex(eventName);
			std::string  include = pyFuncs[index];
			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "generating pyfunc\n";
				outfile << "for event " << eventName << " and func name " << include;
				outfile << "\non line 113 in run event handler run event \n";
				outfile.close();
			}




			result = this->call(eventName, include, dataMap);


			if(result){
				worker.send(eventName, *this);
			}
		}catch(std::exception & e){
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "ERROR: Failed to compile args cause: " << e.what();
			outfile.close();
			exit(1);
		}

		try{
			if (eventName == "ReadyEvent") {
				eventUsed[getIndex(eventName)] = false;
			}
		}
		catch(std::exception & e) {
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile << "\n***************\nrun event line 112 - 123\n*************\n";
			outfile << e.what() << "\n";
			outfile.close();
			exit(1);
		}






		{
		std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\nexiting run event\n*************\n";
		outfile.close();
		}
	}

	int getIndex(std::string eventName){
		std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\ngetIndex\n*************\n";
		int index;
		try{
			index = eventNameMap.at(eventName);
			outfile << "index retrieved: " << index << "\n";
		}
		catch ( std::exception & e){
			outfile << "eventName not found in eventNameMap\nError: " << e.what();
			outfile.close();
			exit(1);
		}
		outfile.close();
		return index;
	}

	bool getIsUsed(std::string eventName){
			 	std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\nGet is used\n*************\n";
		outfile.close();

		bool result = eventUsed[getIndex(eventName)];

		if (result && eventName == "ReadyEvent") {
			eventUsed[getIndex(eventName)] = false;
		}

		return result;
	}

private:
	HapticInterface worker{};


	bool call(std::string eventName, std::string fileName, std::unordered_map<std::string, double> const& dataMap) {
		
		{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "*********************\nIn Call\n ******************\n";
			outfile << "running function for event: " << eventName << "\n";
			outfile.close();
		}
		CPyInstance pyInstance;
		{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "Generating pyobjects\n";
			outfile.close();
		}
		std::vector<CPyObject *> pyObjects;
		{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "Generating Fargs\n";
			outfile.close();
		}
		int index = EventHandler::getIndex(eventName);
		std::vector<RefTypePair> dataRefStrings = eventTypeRefs[index];
		int size =  dataRefStrings.size();
		
		if(size == 0){
			CPyObject pName = PyUnicode_FromString( fileName.c_str());
			if (!pName){
				{
						std::ofstream outfile;
						outfile.open("liuHapticLog.txt", std::ios_base::app);
						outfile << "************ error when loading pName in pyFunction.hpp ************* \n ";
						outfile.close();
				}
				return false;
			}
			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "Generating pModule\n";
				outfile.close();
			}
			CPyObject pModule = PyImport_Import(pName);
			bool res{};
			if (pModule) {
				{
					std::ofstream outfile;
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << " creating Pfunc \n";
					outfile.close();
				}
				//CPyObject pFunc = PyObject_GetAttrString(pModule, "main");

				CPyObject pFunc = PyObject_GetAttrString(pModule, "main");

				{
					std::ofstream outfile;
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "Generating pRes with NULL\n";
					outfile.close();
				}

				if (pFunc && PyCallable_Check(pFunc)) {
					CPyObject pRes = PyObject_CallObject(pFunc.getObject(), NULL);
					res = PyLong_AsLong(pRes);
				}

				{
					std::ofstream outfile;
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << " exiting scope \n";
					outfile.close();
				}
			}
			else {
			#ifdef DEBUG
					std::ofstream outfile;
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << " Error when loading file \n";
					outfile.close();
					exit(1);
			#endif	
			}
			
			

			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "*********************\n Exiting Call\n ******************\n";
				outfile.close();
			}


			for (CPyObject* elem : pyObjects)
			{
				delete elem;
			}


			return res;
		}

		// Everything below is run with args > 0.

		CPyObject Fargs = PyTuple_New(size);

		{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "Generating Pname\n";
			outfile.close();
		}
		CPyObject pName = PyUnicode_FromString( fileName.c_str());
		if (!pName){
			{
					std::ofstream outfile;
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "************ error when loading pName in pyFunction.hpp ************* \n ";
					outfile.close();
			}
			return false;
		}
		{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "Generating pModule\n";
			outfile.close();
		}
		CPyObject pModule = PyImport_Import(pName);

		if (pModule) {
			int counter = 0;
			std::cout << "running python func with values: ";
			
			std::cout << std::endl;
			for (std::pair<std::string, double> x : dataMap) {
				pyObjects.push_back(new CPyObject{PyFloat_FromDouble(x.second)});
			}

			for (CPyObject* elem : pyObjects)
			{

				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "trying to add element to tuple at: " << counter << "\n";
				outfile.close();
				if(Fargs.getObject() == nullptr)
				{
					std::ofstream outfile;
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "Fargs has unexpectadly died on us!!! \n";
					outfile.close();
					exit(1);
				}
				PyTuple_SetItem(Fargs.getObject(), (Py_ssize_t)counter, elem->getObject());
				counter++;

			}


			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << " creating Pfunc \n";
				outfile.close();
			}
			//CPyObject pFunc = PyObject_GetAttrString(pModule, "main");

			CPyObject pFunc = PyObject_GetAttrString(pModule, "main");


			bool res{};
			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "Generating pRes with args\n";
				outfile.close();
			}
			CPyObject pRes = PyObject_CallObject(pFunc.getObject(), Fargs);
			res = PyLong_AsLong(pRes);

			for (CPyObject* elem : pyObjects)
			{
				delete elem;
			}

			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << " exiting scope \n";
				outfile << "*********************\n Exiting Call\n ******************\n";
				outfile.close();
			}

			return res;
		
		}
		else {
		#ifdef DEBUG
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << " Error when loading file \n";
				outfile.close();
				exit(1);
		#endif	
		}
	}
};


	HapticInterface::HapticInterface()
	{
		#ifdef DEBUG
			AllocConsole();
			auto out = freopen("CONOUT$", "w", stdout);
			auto err = freopen("CONOUT$", "w", stderr);
			std::cout << "DEBUG: Active" << std::endl;
		#endif // DEBUG

		std::cout << "creating worker!" << std::endl;
	}

	HapticInterface::~HapticInterface() {
		std::cout << "destroying worker" << std::endl;
	}

	void HapticInterface::send(std::string eventName, EventHandler const& eventHandler) {
		{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "*********************\nInin send\n ******************\n";
			outfile << "running function for event: " << eventName << "\n";
			outfile.close();

			std::cout << "Send - running event for " << eventName << std::endl;
		}
		//int index = eventHandler.eventNameMap.at(eventName) ;
		//std::string tactFileStr = eventFileVec[index];
		//RegisterFeedbackFromTactFile(eventName.c_str(), tactFileStr.c_str());
		//SubmitRegistered(eventName.c_str());
	}

	void HapticInterface::addFileMap(EventToFileVec eventFileVec) {
		std::cout << "adding eventFileMap" << std::endl;
		this->eventFileVec = eventFileVec;
		std::cout << "added" << std::endl;
	}



	/*
	bool call(std::string eventName, std::string fileName, DataRefMap const& dataRefMap) {
		CPyInstance pyInstance;
		CPyObject pFunc;
		XPLMDataRef dataRef;
		CPyObject Fargs;
		int counter = 0;
		std::vector<RefTypePair> dataRefStrings;
		int index = EventHandler::getIndex(eventName);
		dataRefStrings = eventTypeRefs[index];
		
		{
			std::ofstream outfile;
 			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "\nIn CompileArgs\n";

			outfile << "dataRefStrings retreived: " << dataRefStrings.size() << "\n";
			outfile.close();

			if (dataRefStrings.size() != 0) {
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "\nElements in dataRefStrings:\n";
				for (RefTypePair x: dataRefStrings) {
  					outfile << "First: " << x.first << ", Second: " << x.second << "\n";
				}
				outfile.close();
			}
		}
	
		for (RefTypePair refTypePair : dataRefStrings) 
		{
			CPyInstance pyInstance;
			dataRef = dataRefMap.at(refTypePair.second);
			
			std::string valType = refTypePair.first;
			std::variant<int, double> value;
			value = GetValue(dataRef, valType);
			
			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "Value type: " << valType << "\n";
				outfile.close();
			}

			if (valType == "float" || valType == "double") {
				double d_val = std::get<double>(value);
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "Value is: " << d_val << "\n";
				outfile.close();

				PyTuple_SetItem(Fargs, counter,  PyFloat_FromDouble(d_val));
			}
			else {
				long long_val = (long)std::get<int>(value);
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "Value is: " << long_val << "\n";
				outfile.close();
				PyTuple_SetItem(Fargs, counter,  PyLong_FromLong(long_val));
			}
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile << "Converted " << valType << " to CPyObject\n";
			outfile.close();
			counter++;

			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "result created, returning from CompileArgs\n\n";
				outfile.close();
			}

			{
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "************ In Call ************* \n ";
				outfile.close();
			}

		}
		CPyObject pName = PyUnicode_FromString( fileName.c_str());

		if (!pName){
			{
					std::ofstream outfile;
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "************ error when loading pName in pyFunction.hpp ************* \n ";
					outfile.close();
			}
			return false;
		}
		CPyObject pModule = PyImport_Import(pName);

		if (pModule) {
			pFunc = PyObject_GetAttrString(pModule, "main");
	
		}
		else {
		#ifdef DEBUG
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << " Error when loading file \n";
				outfile.close();
				exit(1);
		#endif	
		}

		bool res{};
		CPyObject pRes;
		

		if (counter > 0) {
			if ( pFunc && PyCallable_Check(pFunc))
			{

			#ifdef DEBUG
				std::ofstream outfile;
				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "created tuple ";
				outfile << " before loop \n";
				outfile.close();
			#endif						
				try {
			#ifdef DEBUG
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "after assert before python call\n";
					outfile.close();
			#endif
					pRes = PyObject_CallObject(pFunc, Fargs);
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
				std::ofstream outfile;
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
		{
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile << "returning from pyFunction\n";
			outfile << "Exiting Call \n";
			outfile.close();
		}
		#endif
		return res;
	}
	*/