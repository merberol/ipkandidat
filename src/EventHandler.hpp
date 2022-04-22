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
		CPyInstance pyInstance;
		
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

	void RunEvent(std::string eventName, DataRefMap const& dataRefMap) {
		std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n*************** in EventHandler Run event *************\n";
		outfile.close();
		// first we need to compile arguments
		std::vector<PyVar> args{};
		bool result{};
		try{

			CompileArgs(eventName, dataRefMap, args);
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "args length = " << args.size() << "\n";
			outfile.close();
			
			int index = getIndex(eventName);
			std::string  include = pyFuncs[index];
			outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile << "generating pyfunc\n";
			outfile << "for event " << eventName << " and func name " << include;
			outfile << "\non line 113 in run event handler run event \n";

			int size = args.size();
			outfile << "args still working " << size ;
			outfile.close();
			
			/*-----------------------------------------------
			PyFunc not working, crashes when reading GearUp!
			-----------------------------------------------*/
			PyFunc func = PyFunc(include, size);

			outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile << " loaded file \n";
			outfile.close();	

			result = func.call(args);


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
		return eventUsed[getIndex(eventName)];
	}

private:
	HapticInterface worker{};
	/**
	 * @brief Compiles arguments for pyFunctions
	 * 
	 * @param eventName 
	 * @param dataRefMap 
	 * @return void
	 */
	void CompileArgs(std::string eventName, DataRefMap const& dataRefMap, std::vector<PyVar> & args) {
		CPyInstance pyInstance;
		std::ofstream outfile;
 		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\nIn CompileArgs\n";
		outfile.close();

	
		std::vector<RefTypePair> dataRefStrings;
		int index = EventHandler::getIndex(eventName);

		try{
			dataRefStrings = eventTypeRefs[index];
			outfile.open("liuHapticLog.txt", std::ios_base::app);
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
		catch (std::exception & e){
			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "no more dataRefStrings\n";
			outfile.close();
			exit(1);
		}

		int counter = 0;
		XPLMDataRef dataRef;
		try{
			for (RefTypePair refTypePair : dataRefStrings) {
				try{
					dataRef = dataRefMap.at(refTypePair.second);
				}
				catch (std::exception & e) {
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "Failed to get data ref from map cause: " << e.what();
					outfile.close();
					exit(1);
				}
				std::string valType = refTypePair.first;
				std::variant<int, double> value;
				
				try{
					value = GetValue(dataRef, valType);
				}catch( std::exception & e){
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "Failed to get value from dataref cause: " << e.what();
					outfile.close();
					exit(1);
				}

				outfile.open("liuHapticLog.txt", std::ios_base::app);
				outfile << "Value type: " << valType << "\n";
				outfile.close();

				try{
					if (valType == "float" || valType == "double") {
						double d_val = std::get<double>(value);
						outfile.open("liuHapticLog.txt", std::ios_base::app);
						outfile << "Value is: " << d_val << "\n";
						outfile.close();
						args.push_back(PyVar{d_val});
					}
					else {
						long long_val = (long)std::get<int>(value);
						outfile.open("liuHapticLog.txt", std::ios_base::app);
						outfile << "Value is: " << long_val << "\n";
						outfile.close();
						args.push_back(PyVar{long_val});
					}
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "Converted " << valType << " to CPyObject\n";
					outfile.close();
					counter++;
				}catch (std::exception & e) {
					outfile.open("liuHapticLog.txt", std::ios_base::app);
					outfile << "Failed to convert value to CPObject cause: " << e.what();
					outfile.close();
					exit(1);
				}
			}

			outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile <<  "size of args: "<< args.size() << " which should be: "<< counter  << "\n";
			outfile.close();
		}
		catch (std::exception & e){
			exit(1);
		}
		outfile.open("liuHapticLog.txt", std::ios_base::app);
		outfile << "result created, returning from CompileArgs\n\n";
		outfile.close();
		return;
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