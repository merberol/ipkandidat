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
#include "Logger.hpp"
#include "XPLMDataAccess.h"



class EventHandler;

class HapticInterface {
	EventToFileVec eventFileVec{};
public:
	HapticInterface();
	~HapticInterface();
	void addFileVec(EventToFileVec & eventFileVec);
	void sendToVest(std::string const& eventName, EventHandler const& eventHandler) const;
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
	
	
	EventHandler() = default;

	EventHandler(std::string id) 
		:_id{ id }
	{
		StreamLogger::log("EventHandler : Constructor", "liuHapticLog.txt", "############# Instansiating EventHandler ##############");
		char playerPath[200];
		int esize = 2;
		bool res = TryGetExePath(playerPath, esize);
		auto path = GetExePath();

		if (!res) {
			std::stringstream output{};
			output << "Cannot find exe path for Bhaptics Player.";
			StreamLogger::log("EventHandler : Constructor", "liuHapticLog.txt", output);
			exit(1);
		}
		// initialising bHapticPlayer link
		Initialise(id.c_str(), "LiuXPlaneHapticPlugin");
		TactFileVec tactFiles{};
		ConfigLoader configLoader{};
		configLoader.run(eventNameMap, eventUsed, tactFiles, refPathVec, eventTypeRefs, pyFileNames);

		worker.addFileVec(tactFiles);
	};

	~EventHandler() {
		Destroy();
		StreamLogger::log("EventHandler : Destructor", "liuHapticLog.txt", "############# Destroying EventHandler ##############");
	}

	void runEvent(std::string eventName, std::unordered_map<std::string, double> const& dataMap) {
		bool result{};
#ifdef DEBUG
		{
			std::stringstream output{};
			output << "Running Event: " << eventName;

			StreamLogger::log("EventHandler : Destructor", "liuHapticLog.txt", output);
		}
#endif
		try{
			int index = getIndex(eventName);
			std::string  include = pyFileNames[index];
			result = this->call(eventName, include, dataMap);

			if(result){
				worker.sendToVest(eventName, *this);
			}
		}
		catch(std::exception & e){
			std::stringstream output{};
			output << "ERROR: Failed to compile args cause: " << e.what();
			StreamLogger::log("EventHandler : runEvent", "liuHapticLog.txt", output);
			exit(1);
		}

		try{
			if (eventName == "ReadyEvent") {
				eventUsed[getIndex(eventName)] = false;
			}
		}
		catch(std::exception & e) {
			std::stringstream output{};
			output << "Error: " << e.what();
			StreamLogger::log("EventHandler : runEvent", "liuHapticLog.txt", output);
			exit(1);
		}
	}

	int getIndex(std::string const& eventName) const{
		int index;
#ifdef DEBUG
		{
			std::stringstream output{};
			output << "getting Index for " << eventName;

			StreamLogger::log("EventHandler : getIndex", "liuHapticLog.txt", output);
		}
#endif
		try{
			index = eventNameMap.at(eventName);
		}
		catch ( std::exception & e){
			std::stringstream output{};
			output << "EventName not found in eventNameMap\nError: " << e.what();
			StreamLogger::log("EventHandler :getIndex", "liuHapticLog.txt", output);
			exit(1);
		}
#ifdef DEBUG
		{
			std::stringstream output{};
			output << "returning index: " << index;

			StreamLogger::log("EventHandler : getIndex", "liuHapticLog.txt", output);
		}
#endif
		return index;
	}

	bool getIsUsed(std::string const& eventName) {
#ifdef DEBUG
		{
			std::stringstream output{};
			output << "Getting if event " << eventName << "is used";

			StreamLogger::log("EventHandler : getIsUsed", "liuHapticLog.txt", output);
		}
#endif
		bool result = eventUsed[getIndex(eventName)];

		if (result && eventName == "ReadyEvent") {
			eventUsed[getIndex(eventName)] = false;
		}
#ifdef DEBUG
		{
			std::stringstream output{};
			output << "returning result " << result;
			StreamLogger::log("EventHandler : getIsUsed", "liuHapticLog.txt", output);
		}
#endif
		return result;
	}

private:
	HapticInterface worker{};

	bool call(std::string eventName, std::string fileName, std::unordered_map<std::string, double> const& dataMap) {
	#ifdef DEBUG
		{
			std::stringstream output{};
			output << "Calling Python logic for event: " << eventName;

			StreamLogger::log("EventHandler : call", "liuHapticLog.txt", output);
		}
#endif
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

				for (RefTypePair x : dataRefStrings) {
					pyObjects.push_back(new CPyObject{PyFloat_FromDouble(dataMap.at(x.second))});
				}
				
				for (CPyObject* elem : pyObjects)
				{
					if(funcArgs.getObject() == nullptr)
					{
						{
							std::stringstream output{};
							output << "FuncArgs contains nothing.";
							StreamLogger::log("EventHandler : call", "liuHapticLog.txt", output);
						}
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
			std::stringstream output{};
			output << " Error when loading file: ";
			StreamLogger::log("EventHandler : call", "liuHapticLog.txt", output);
			exit(1);
		}

		for (CPyObject* elem : pyObjects)
		{
			delete elem;
		}

		return result;
	}
};

/*
 / ---------------------------------------------------------------------------------------------------------------------------------------------
 / ------------------------------------------------ Haptic Interface Implementation ------------------------------------------------------------
 / ---------------------------------------------------------------------------------------------------------------------------------------------
*/

HapticInterface::HapticInterface()
{
#ifdef DEBUG
	{
		std::stringstream output{};
		output << "### Creating HapticInterface ###";
		StreamLogger::log("EventHandler : Constructor", "liuHapticLog.txt", output);
	}
#endif
}

HapticInterface::~HapticInterface() {
#ifdef DEBUG
	{
		std::stringstream output{};
		output << "### Destroying HapticInterface ###";

		StreamLogger::log("EventHandler : Destructor", "liuHapticLog.txt", output);
	}
#endif
}

/**
 * @brief Adds a event file map to the in
 * 
 * @param eventFileVec 
 */
void HapticInterface::addFileVec(EventToFileVec & eventFileVec) {
	if( this->eventFileVec.empty() ){
		this->eventFileVec = std::move(eventFileVec);
	}
	else{
		StreamLogger::log(" HapticInterface : addFileVec", "liuHapticLog.txt","ERROR: Trying to overwrite eventFileMap during run time");
		exit(1);

	}
}

void HapticInterface::sendToVest(std::string const& eventName, EventHandler const& eventHandler) const {
#ifdef DEBUG
	{
		std::stringstream output{};
		output << "running function for event: " << eventName ;
		StreamLogger::log("EventHandler : sendToVest", "liuHapticLog.txt", output);
	}
#endif
	int index = eventHandler.getIndex(eventName) ;
	std::string tactFileStr = eventFileVec[index];
	RegisterFeedbackFromTactFile(eventName.c_str(), tactFileStr.c_str());
	SubmitRegistered(eventName.c_str());
	
}