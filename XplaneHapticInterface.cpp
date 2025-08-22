
/**
 * @file XplaneHapticInterface.cpp
 * @author Charlie Simonsson simonsson.charlie@gmail.com & Marcus Franzén
 * @brief 
 * Compile command: cl /EHsc /std:c++17 /Fe:"C:\X-Plane 11\Resources\plugins\LiuHaptics\win.xpl" /I includes\Xplane\CHeaders\XPLM /I includes\Python39 /I includes\bHaptics /I src XplaneHapticInterface.cpp /LD /INCREMENTAL:NO /link /LIBPATH:"C:\XplHaptInterface\libs"
 * Use this command line comand with the x64 Tools Command Prompt for vs 2022 to compile the program. make shure the system has xplane installed at c: 
 * 
 * @version 1
 * @date 2022-05-23
 * 
 * @copyright Copyright (c) 2022
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
#pragma comment(lib,"python39_d.lib")
#pragma comment(lib,"Xplane\\XPLM_64.lib")
#pragma comment(lib,"bHapticSDK\\bin\\win64\\haptic_library.lib")

//#define WINVER 0x0601
//#define _WIN32_WINNT 0x0601
#define _WIN32_WINDOWS 0x0601
#define WIN32
#define NDEBUG
#define _WINDOW
#define _USRDLL
#define SIMDATA_EXPORTS
#define IBM 1
#define XPLM200 1
#define XPLM210 1
#define _CRT_SECURE_NO_WARNINGS
#define _VC80_UPGRADE 0x0600
#define _WINDLL
#define _MBCS





#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <memory>
#include "src/EventHandler.hpp"
#include "src/Logger.hpp"
#include <chrono>
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"



static DataRefMap dataRefMap{};
static std::unordered_map<std::string, double> dataMap;
EventHandler* eventHandler;

#ifdef TIME_CHECK
namespace HI {

static double loadtime{};
static std::vector<double> flightLoopData{};
static std::unordered_map<std::string, std::vector<double>*> eventTimeData{};
}
#endif

static float	HapticFlightLoopCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
                                   int                  inCounter,    
                                   void *               inRefcon);    


// cc 2 loc 14
PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	strcpy_s(outName, 18, "LIU Haptic Plugin");
	strcpy_s(outSig, 18, "liu.haptic_plugin");
	strcpy_s(outDesc, 55, "A plugin for testing a haptic vest with the flightsim.");

	eventHandler = new EventHandler("se.liu.haptic_plugin");

	remove("liuHapticLog.txt");
	

#ifdef DEBUG
	StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt",  "\n***************\nBuilding dataRefMap\n***************\n");
#endif

	auto loadStart = std::chrono::system_clock::now();

	// dataref strings required, get from config loader via EventHandlers interface
	for (int i = 0; i < eventHandler->refPathVec.size(); i++) {
		XPLMDataRef value = XPLMFindDataRef(eventHandler->refPathVec[i].c_str());

#ifdef DEBUG
		std::stringstream output{};
		output << "Emplacing key: " << eventHandler->refPathVec[i] << " with value: " << value;
		StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
		dataRefMap.emplace(eventHandler->refPathVec[i], value);
	}
	auto loadEnd = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed = loadEnd - loadStart;

// for time testing purposes.
#ifdef TIME_CHECK
	HI::loadtime = elapsed.count();
#endif

#ifdef DEBUG
	std::stringstream output{};
	output <<  "Done\n"
		<< "Loading took " << elapsed.count() << "seconds\n***************\n";
	StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
	XPLMRegisterFlightLoopCallback(
		HapticFlightLoopCallback,	/* Callback */
		1.0,						/* Interval */
		NULL);						/* refcon not used. */

	return 1;
}

 // cc 4 loc 26
/**
 * @brief Retrives the data map for the current event.
 * 
 * To exted this method add another valtype case as a elseif block and use the apropriate XPLMGetData method and convert the result to a float.
 * 
 * @param eventName 
 * @return std::unordered_map<std::string, double> 
 * 
 */
std::unordered_map<std::string, double> getData(std::string eventName) {
	// TODO: move this to types or EventHandler and add datarefmap and eventhadler references to parameter list to avoid assuming globals
	std::vector<RefTypePair> dataRefStrings = eventHandler->eventTypeRefs[eventHandler->getIndex(eventName)];
	std::unordered_map<std::string, double> data{};
	int counter = 0;

	for (RefTypePair refTypePair : dataRefStrings) {
		XPLMDataRef dataRef = dataRefMap.at(refTypePair.second);
		std::string valType = refTypePair.first;

		if (valType == "int") {
			int dataValue = XPLMGetDatai(dataRef);
#ifdef DEBUG
			std::stringstream output{};
			output << valType << " " << refTypePair.first << ": " << dataValue;
			StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
			data.emplace(refTypePair.second, dataValue);
		}
		else if (valType == "float") {
			float dataValue = XPLMGetDataf(dataRef);
#ifdef DEBUG
			std::stringstream output{};
			output << valType << " " << refTypePair.first << ": " << dataValue;
			StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
			data.emplace(refTypePair.second, dataValue);
		}
		else if (valType == "double") {
			double dataValue = XPLMGetDatad(dataRef);
#ifdef DEBUG
			std::stringstream output{};
			output << valType << " " << refTypePair.first << ": " << dataValue;
			StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
			data.emplace(refTypePair.second, dataValue);
		}
		else {
			std::stringstream output{};
			output << "Unusable value type in config: " << valType << "\n";
			output << "Use only int, float or double.";
			StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
			// exit(1);
			throw std::runtime_error("Unusable value type in config: " + valType + "\nUse only int, float or double.");
		}
		counter++;
	}

	std::cout << std::endl;
	counter = 0;
	return data;
}


// cc 3 loc 15
float	HapticFlightLoopCallback(
                                   float                inElapsedSinceLastCall,
                                   float                inElapsedTimeSinceLastFlightLoop,
                                   int                  inCounter,
                                   void *               inRefcon)
{
	std::stringstream output{};
#ifdef TIME_CHECK
	auto flStart = std::chrono::system_clock::now();
#endif

#ifdef DEBUG
	output << "****************************************************************************************************\n"
		<< "Flight Loop start\n";
	StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
	try {
		for (EventIndexPair p : eventHandler->eventNameMap) {

#ifdef TIME_CHECK
			auto elStart = std::chrono::system_clock::now();
#endif

#ifdef DEBUG
			output.clear();
			output << "****************************************************************************\n"
				<< "Event Loop Start\n";
			StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
			if (eventHandler->isUsed(p.first)) {
				// TODO: Konsider removing this from global scope and instead instansiate here where it is needed and pass as reference to getData..
				dataMap = getData(p.first);
				eventHandler->runEvent(p.first, dataMap);
			}
#ifdef TIME_CHECK
			auto elEnd = std::chrono::system_clock::now();
			std::chrono::duration<double> ELTime = elEnd - elStart;
			if(HI::eventTimeData.find(p.first) == HI::eventTimeData.end()){
				HI::eventTimeData.emplace(p.first, new std::vector<double>{});
			}
			HI::eventTimeData.at(p.first)->push_back(ELTime.count());

#endif
#ifdef DEBUG
			output.clear();
			output << "Event Loop End"
#ifdef TIME_CHECK
				<< " with run time: " << ELTime.count() << " seconds" 
#endif
				<< "\n****************************************************************************";
			StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
		}
	}
	catch (std::exception & e) {
		std::cout << e.what() << std::endl;
		std::stringstream output{};
		output << "Unexpected exception:\n" 
			<< e.what()
			<< "\nin Flightloop.";
		StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#ifdef DEBUG
		// if we are in debug mode we want to log the error and crash X-plane
		// to make shure the plugin is not running in a bad state, while developing or 
		// testing the plugin. 
		exit(1);
#else
		
		// Returning 0.0f from the flight loop callback unregisters it
		// and prevents it from being called again. This is a graceful
		// way to disable the plugin on a critical error instead of crashing X-Plane.
		return 0.0f;
#endif
	}
#ifdef TIME_CHECK
	auto flEnd = std::chrono::system_clock::now();
	std::chrono::duration<double> FLTime = flEnd - flStart;
	HI::flightLoopData.push_back(FLTime.count());

#endif
#ifdef DEBUG
	output.clear();
	output << "Fligh Loop end"
#ifdef TIME_CHECK
		<< " with run time: " << FLTime.count() << " seconds"
#endif
		<< "\n****************************************************************************************************\n";
	StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
#endif
	// Return 1.0 to indicate that we want this function to be called again in 1 second.
	return 1.0;
}

#ifdef TIME_CHECK
///////////////// not strictly a part of the system unless you count debug and data collection
// cc 2 loc 5
void saveData(std::string fileName, std::vector<double> const& data, std::string columnName){
	std::stringstream output{columnName +";\n"};
	for(double time: data){
		output << time << ";\n";
	}
	StreamLogger::lograw(fileName, output.str());
}
// cc1 loc 2
void buildFlightLoopFile(){
	saveData("flightloopdata.csv", HI::flightLoopData, "loopTimes");
}

// cc 2 loc 3
void buildEventTimesFile(){
	
	for( auto eventPair : HI::eventTimeData) {
		saveData( eventPair.first+"data.csv", *eventPair.second, eventPair.first);
	}
}

// cc 1 loc 8
void saveData()
{
	buildFlightLoopFile();
	buildEventTimesFile();
	saveData("TopCallTimes.csv", eventHandler->TopLevelCallTimer, "calltimes");
	saveData("TopSendTimes.csv", eventHandler->TopLevelSendTimer, "sendtimes");
	saveData("PyImportTimes.csv", eventHandler->PyImportTimes, "importTimes");
	saveData("PyParseTimes.csv", eventHandler->PyArgParseTimes, "parseTimes");
	saveData("PyCallTimes.csv", eventHandler->PyCallTimes, "pycallTimes");
}

/////////////////////////////////////////////////////////////////
#endif
// cc 1 loc 4
PLUGIN_API void	XPluginStop(void){
#ifdef DEBUG
	StreamLogger::log("XplaneHapticInterface : XPluginStop", "liuHapticLog.txt", "shutting down plugin");
#endif
#ifdef TIME_CHECK
	saveData();
#endif

	// Destroy allocated resources.
	delete eventHandler;
	eventHandler = nullptr;

	XPLMUnregisterFlightLoopCallback(HapticFlightLoopCallback, NULL);
#ifdef DEBUG
	StreamLogger::log("XplaneHapticInterface : XPluginStop", "liuHapticLog.txt", "exiting scope");
#endif
}

// cc 1 loc 1
PLUGIN_API void XPluginDisable(void) {}
// cc 1 loc 1
PLUGIN_API int XPluginEnable(void) { return 1; }
// cc 1 loc 1
PLUGIN_API void XPluginReceiveMessage(
	XPLMPluginID	inFromWho,
	int				inMessage,
	void* inParam)
{}