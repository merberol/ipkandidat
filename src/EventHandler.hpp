/**
 * @file EventHandler.hpp
 * @author Charlie Simonsson simonsson.charlie@gmail.com & Marcus Franzén
 * @brief 
 * @version 1
 * @date 2022-05-23
 * 
 * @copyright Copyright (c) 2022
 * 
 * Authors: 
 * Licence
 * The MIT License (MIT)
 *
 * Copyright (c) <2022> <Charlie simonsson & Marcus Franzén>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *~
 * ~~~~~~~~ Change LOG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *~
 * ~~ Add and record of any changes and bug fixes to the system in this section
 * ~~ of the file where those changes where made.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *~
 * may 23 2022: Added Licence and change log : Charlie
 * 
 */
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
#include <chrono>
#include "types.h"
#include "ConfigLoader.hpp"
#include "includes\bHaptics\HapticLibrary.h"
#include "Logger.hpp"
#include "XPLMDataAccess.h"


/**
 * @brief the Main interface coupling the X-Plane SDK ( see XplaveHapticInterface.cpp)
 * and the bHaptics SDK ( see the Haptic interface class below ) into a central system that generates and 
 * dispatches haptic activation to the vest.
 * 
 * -- External Dependensies
 * 		the system needs to have the bHaptics player up and running before instansiating this class
 * 		The best way to enshure this is by starting bHaptics Player before starting X-Plane
 * 
 */
class EventHandler;

/**
 * @brief sends data to the bHaptics Player program
 * 
 * 		
 * 
 */
class HapticInterface {
	EventToFileVec eventFileVec{};
public:
	/**
	 * @brief Construct a new Haptic Interface object 
	 * 		contains debug code
	 * 
	 */
	HapticInterface();

	/**
	 * @brief Destroy the Haptic Interface object
	 * 		contains debug code
	 * 
	 */
	~HapticInterface();

	/**
 	* @brief Adds a event file map to the in
 	* 
 	* @param eventFileVec 
 	*/
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

#ifdef TIME_CHECK
	std::vector<double> TopLevelCallTimer{};
	std::vector<double> TopLevelSendTimer{};
	std::vector<double> PyImportTimes{};
	std::vector<double> PyArgParseTimes{};
	std::vector<double> PyCallTimes{};
#endif
	
	// cc 1 loc 1
	EventHandler() = default;

	// cc 2 loc 16
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

	// cc 1 loc 3
	~EventHandler() {
		Destroy();
		StreamLogger::log("EventHandler : Destructor", "liuHapticLog.txt", "############# Destroying EventHandler ##############");
	}

	// cc 3 loc 21
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
#ifdef TIME_CHECK
			auto beforeCall = std::chrono::system_clock::now();
#endif
			result = this->call(eventName, include, dataMap);
#ifdef TIME_CHECK
			auto afterCall = std::chrono::system_clock::now();
#endif
			if(result){
				worker.sendToVest(eventName, *this);
			}
#ifdef TIME_CHECK
			auto aftersend = std::chrono::system_clock::now();
 
			std::chrono::duration<double> calltime = afterCall - beforeCall;
			std::chrono::duration<double> sendtime = aftersend - afterCall;
			TopLevelCallTimer.push_back(calltime.count());
			TopLevelSendTimer.push_back(sendtime.count());
#endif
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

 	// cc 1 loc 10
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

	// cc 2 loc 5
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

	// cc 9 loc 39
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

#ifdef TIME_CHECK
			auto beforeImport = std::chrono::system_clock::now();
#endif
		// importing py module and function 
		CPyObject pModule = PyImport_Import(pName);
		CPyObject pFunc = PyObject_GetAttrString(pModule, "main");
#ifdef TIME_CHECK
			auto afterImport = std::chrono::system_clock::now();
			std::chrono::duration<double> importtime = afterImport - beforeImport;
			PyImportTimes.push_back(importtime.count());
#endif

		if (pModule) {
			if (size == 0) {
				if (pFunc && PyCallable_Check(pFunc)) {
					CPyObject pRes = PyObject_CallObject(pFunc.getObject(), NULL);
					result = PyLong_AsLong(pRes);
				}
			}
			else {
#ifdef TIME_CHECK
				auto beforePyCall = std::chrono::system_clock::now();
#endif
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
#ifdef TIME_CHECK
				auto afterArgParse = std::chrono::system_clock::now();
#endif
				if (pFunc && PyCallable_Check(pFunc)) {
					CPyObject pRes = PyObject_CallObject(pFunc.getObject(), funcArgs);
					result = PyLong_AsLong(pRes);
				}
#ifdef TIME_CHECK
				auto afterPyCall = std::chrono::system_clock::now();
				std::chrono::duration<double> argparseTime = afterArgParse - beforePyCall;
				std::chrono::duration<double> pycallTime = afterPyCall - afterArgParse;
				PyArgParseTimes.push_back(argparseTime.count());
				PyCallTimes.push_back(pycallTime.count());
#endif
			}
		}
		else {
			std::stringstream output{};
			output << " Error when loading file: ";
			StreamLogger::log("EventHandler : call", "liuHapticLog.txt", output);
			exit(1);
		}
#ifdef TIME_CHECK
				auto beforeDel = std::chrono::system_clock::now();
#endif
		for (CPyObject* elem : pyObjects)
		{
			delete elem;
		}
#ifdef TIME_CHECK
				auto afterDel = std::chrono::system_clock::now();
				std::chrono::duration<double> DelTime = afterDel - beforeDel;
				std::stringstream output{};
				output << "call delete took: " << DelTime.count() << "seconds";
				StreamLogger::log("EventHandler : call", "liuHapticLog.txt", output);

#endif
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

void HapticInterface::addFileVec(EventToFileVec & eventFileVec) {
	if( this->eventFileVec.empty() ){
		this->eventFileVec = std::move(eventFileVec);
	}
	else{
		StreamLogger::log(" HapticInterface : addFileVec", "liuHapticLog.txt","ERROR: Trying to overwrite eventFileMap during run time, action refused");
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