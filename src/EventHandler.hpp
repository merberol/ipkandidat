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
	EventNameMap eventNameMap;
	EventUsedVec eventUsed;

	std::vector<std::vector<RefTypePair>> eventTypeRefs;
	RefPathVector refPathVec;
	PyFileNameVec pyFuncs;
	
	EventHandler() = default;

	EventHandler(std::string id) 
		:_id{ id }
	{
		CPyInstance pyInstance;
		
		char playerPath[200];
		int size = 2;
		bool res = TryGetExePath(playerPath, size);
		auto path = GetExePath();

		if (res) {
			std::cout << "1. getExePath bHaptics Player is installed:  " << playerPath << std::endl << path << std::endl;
			std::cout << "1. player path size:  " << size << std::endl;

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

		ResultType result = configLoader.run();

		eventNameMap = std::get<0>(result);
		eventUsed = std::get<1>(result);
		TactFileVec tactFiles = std::get<2>(result);
		refPathVec = std::get<3>(result);
		eventTypeRefs = std::get<4>(result);
		pyFuncs = std::get<5>(result);

		worker.addFileMap(tactFiles);
	};

	~EventHandler() {
		Destroy();
	}

	void RunEvent(std::string eventName, DataRefMap const& dataRefMap) {
			 	std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\nRun eventp\n*************\n";
		outfile.close();
		// first we need to compile arguments
		std::vector<CPyObject> args = CompileArgs(eventName, dataRefMap);
	

		bool result;
		int index = getIndex(eventName);
		
		PyFunc func{pyFuncs[index], (int)args.size()};
		result = func.call(args);

		
		
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
		if(result){
			worker.send(eventName, *this);
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
		outfile.close();
		return eventNameMap.at(eventName);
	}

	bool getIsUsed(std::string eventName){
			 	std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\nGet is usedp\n*************\n";
		outfile.close();
		return eventUsed[getIndex(eventName)];
	}

private:
	HapticInterface worker{};



	std::vector<CPyObject> CompileArgs(std::string eventName, DataRefMap const& dataRefMap) {
		std::ofstream outfile;
 		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n\nIn CompileArgs\n";
		std::vector<CPyObject> result{};
		int index;
		try{
			index = eventNameMap.at(eventName);
		}
		catch ( std::exception & e){
			outfile.close();
			exit(1);
		}

		outfile << "index retrieved " << index <<  "\n";

		std::vector<RefTypePair> dataRefStrings;
		try{
			dataRefStrings = eventTypeRefs[index];
			outfile << "dataRefStrings retreived" << dataRefStrings.size() << "\n";
		}
		catch (std::exception & e){
			std::cout << "no dataRefStrings found at index " << index << std::endl;  
			outfile << "no more dataRefStrings\n";
			exit(1);
		}
		int counter = 0;
		try{
			for (RefTypePair refTypePair : dataRefStrings) {
				outfile << "ref type first " << refTypePair.first << " ref type second: " << refTypePair.second << "\n";
				XPLMDataRef dataRef = dataRefMap.at(refTypePair.second);
				std::string valType = refTypePair.first;
				std::variant<int, double> value = GetValue(dataRef, valType);

				if (valType == "float" || valType == "double") 
				{
					CPyObject val = PyFloat_FromDouble(std::get<double>(value));
					result.push_back(val);
				}
				else 
				{
					CPyObject val = PyLong_FromLong((long)std::get<int>(value));
					result.push_back(val);
				}
				counter++;
			}
			outfile <<  "size of args: "<< result.size() << "wich shuld be: "<< counter  << "\n";
		}
		catch (std::exception & e){
			outfile.close();
			exit(1);
		}
		outfile << "result created, returning from CompileArgs\n\n";
		outfile.close();
		return result;
	}
	
	std::variant<int, double> GetValue(XPLMDataRef dataRef, std::string valType)
	{
		if (valType == "float" || valType == "double") {
			return XPLMGetDatad(dataRef);
		}
		return XPLMGetDatai(dataRef);
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
		int index = eventHandler.eventNameMap.at(eventName) ;
		std::string tactFileStr = eventFileVec[index];
		RegisterFeedbackFromTactFile(eventName.c_str(), tactFileStr.c_str());
		SubmitRegistered(eventName.c_str());
	}

	void HapticInterface::addFileMap(EventToFileVec eventFileVec) {
		std::cout << "adding eventFileMap" << std::endl;
		this->eventFileVec = eventFileVec;
		std::cout << "added" << std::endl;
	}