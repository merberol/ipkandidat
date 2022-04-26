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
	void addFileMap(EventToFileVec eventFileVec);
	void sendToVest(std::string eventName, EventHandler const& eventHandler);
};

class EventHandler
{

public:
	std::string _id;
	EventNameMap eventNameMap{};
	EventUsedVec eventUsed{};
	std::vector<std::vector<RefTypePair>> eventTypeRefs{};
	RefPathVector refPathVec{};
	PyFileNameVec pyFileNames{};
	TactFileVec tactFiles{};
	
	EventHandler() = default;

	EventHandler(std::string id) 
		:_id{ id }
	{
		std::cout << "### Creating EventHandler ###" << std::endl;
		char playerPath[200];
		int esize = 2;
		bool res = TryGetExePath(playerPath, esize);
		auto path = GetExePath();

		if (res) {
			std::cout << "GetExePath bHaptics Player is installed: " << playerPath << std::endl << path << std::endl;
			std::cout << "Player path size: " << esize << std::endl;
		}
		else {
			std::cout << "Cannot find exe path." << std::endl;
		}

		startBHapticsPlayer(id);
		ConfigLoader configLoader{};
		configLoader.run(eventNameMap, eventUsed, tactFiles, refPathVec, eventTypeRefs, pyFileNames);
		worker.addFileMap(tactFiles);
	};

	~EventHandler() {
		Destroy();
		std::cout << "### Destroying EventHandler ###" << std::endl;
	}

	void startBHapticsPlayer(std::string id) {
		std::cout << "Starting initialization." << std::endl;
		auto start = std::chrono::system_clock::now();
		Initialise(id.c_str(), "LiuXPlaneHapticPlugin");
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << "Initialization elapsed time: " << elapsed_seconds.count() << "s\n";
	}

	void runEvent(std::string eventName, std::unordered_map<std::string, double> const& dataMap) {
		bool result{};
		try{
			int index = getIndex(eventName);
			std::string  include = pyFileNames[index];
			result = this->call(eventName, include, dataMap);

			if(result){
				worker.sendToVest(eventName, *this);
			}
		}
		catch(std::exception & e){
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
			outfile << e.what() << "\n";
			outfile.close();
			exit(1);
		}
	}

	int getIndex(std::string eventName){
		int index;
		try{
			index = eventNameMap.at(eventName);
		}
		catch ( std::exception & e){
			std::ofstream outfile;
			outfile << "EventName not found in eventNameMap\nError: " << e.what();
			outfile.close();
			exit(1);
		}
		return index;
	}

	bool getIsUsed(std::string eventName){
		bool result = eventUsed[getIndex(eventName)];

		if (result && eventName == "ReadyEvent") {
			eventUsed[getIndex(eventName)] = false;
		}

		return result;
	}

private:
	HapticInterface worker{};

	bool call(std::string eventName, std::string fileName, std::unordered_map<std::string, double> const& dataMap) {

		CPyInstance pyInstance;
		std::vector<CPyObject *> pyObjects;
		int index = EventHandler::getIndex(eventName);
		std::vector<RefTypePair> dataRefStrings = eventTypeRefs[index];
		int size = dataRefStrings.size();
		CPyObject pName = PyUnicode_FromString(fileName.c_str());
		bool result{};
		
		if (!pName) {
			return false;
		}

		CPyObject pModule = PyImport_Import(pName);
		CPyObject pFunc = PyObject_GetAttrString(pModule, "main");

		if (pModule) {
			if (size == 0) {
				if (pFunc && PyCallable_Check(pFunc)) {
					CPyObject pRes = PyObject_CallObject(pFunc.getObject(), NULL);
					result = PyLong_AsLong(pRes);
				}
			}
			else {
				CPyObject funcArgs = PyTuple_New(size);
				int counter = 0;

				for (std::pair<std::string, double> x : dataMap) {
					pyObjects.push_back(new CPyObject{PyFloat_FromDouble(x.second)});
				}
				
				for (CPyObject* elem : pyObjects)
				{
					if(funcArgs.getObject() == nullptr)
					{
						std::ofstream outfile;
						outfile.open("liuHapticLog.txt", std::ios_base::app);
						outfile << "FuncArgs contains nothing. \n";
						outfile.close();
						exit(1);
					}

					PyTuple_SetItem(funcArgs.getObject(), (Py_ssize_t)counter, elem->getObject());
					counter++;
				}

				if (pFunc && PyCallable_Check(pFunc)) {
					CPyObject pRes = PyObject_CallObject(pFunc.getObject(), funcArgs);
					result = PyLong_AsLong(pRes);
				}
			}
		}
		else {
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile << " Error when loading file \n";
			outfile.close();
			exit(1);
		}

		for (CPyObject* elem : pyObjects)
		{
			delete elem;
		}

		return result;
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

	std::cout << "### Creating HapticInterface ###" << std::endl;
}

HapticInterface::~HapticInterface() {
	std::cout << "### Destroying HapticInterface ###" << std::endl;
}

void HapticInterface::addFileMap(EventToFileVec eventFileVec) {
	this->eventFileVec = eventFileVec;
}

void HapticInterface::sendToVest(std::string eventName, EventHandler const& eventHandler) {
	{
		std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
		outfile << "*********************\nInin send\n ******************\n";
		outfile << "running function for event: " << eventName << "\n";
		outfile.close();

		std::cout << "Send - running event for " << eventName << std::endl;
	}
	int index = eventHandler.eventNameMap.at(eventName) ;
	std::string tactFileStr = eventFileVec[index];
	RegisterFeedbackFromTactFile(eventName.c_str(), tactFileStr.c_str());
	SubmitRegistered(eventName.c_str());
}