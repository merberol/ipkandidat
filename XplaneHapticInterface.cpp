
// cl /EHsc /std:c++17 /Fe:"C:\X-Plane 11\Resources\plugins\LiuHaptics\win.xpl" /I includes\Xplane\CHeaders\XPLM /I includes\Python39 /I includes\bHaptics /I src XplaneHapticInterface.cpp /LD /INCREMENTAL:NO /link /LIBPATH:"C:\XplHaptInterface\libs"
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
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"


static DataRefMap dataRefMap{};
static std::unordered_map<std::string, double> dataMap;
EventHandler* eventHandler;

static float	HapticFlightLoopCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
                                   int                  inCounter,    
                                   void *               inRefcon);    


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
#ifdef DEBUG
	StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt",  "Done\n***************\n");
#endif
	XPLMRegisterFlightLoopCallback(
		HapticFlightLoopCallback,	/* Callback */
		1.0,						/* Interval */
		NULL);						/* refcon not used. */

	return 1;
}

/**
 * @brief Retrives the data map for the current event.
 * 
 * To exted this method add another valtype case as a elseif block and use the apropriate XPLMGetData method and convert the result to a float.
 * 
 * @param eventName 
 * @return std::unordered_map<std::string, double> 
 */
std::unordered_map<std::string, double> getData(std::string eventName) {
	std::vector<RefTypePair> dataRefStrings = eventHandler->eventTypeRefs[eventHandler->getIndex(eventName)];
	std::unordered_map<std::string, double> data{};
	int counter = 0;

	for (RefTypePair refTypePair : dataRefStrings) {
		XPLMDataRef dataRef = dataRefMap.at(refTypePair.second);
		std::string valType = refTypePair.first;

		if (valType == "int") {
			long dataValue = XPLMGetDatai(dataRef);
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
			exit(1);
		}
		counter++;
	}

	std::cout << std::endl;
	counter = 0;
	return data;
}

float	HapticFlightLoopCallback(
                                   float                inElapsedSinceLastCall,
                                   float                inElapsedTimeSinceLastFlightLoop,
                                   int                  inCounter,
                                   void *               inRefcon)
{
	std::stringstream output{};
	output << "****************************************************************************************************\n"
		<< "Fligh Loop start";
	StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
	try {
		for (EventIndexPair p : eventHandler->eventNameMap) {
			output.clear();
			output << "****************************************************************************\n"
				<< "Event Loop Start";
			StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
			if (eventHandler->getIsUsed(p.first)) {
				dataMap = getData(p.first);
				eventHandler->runEvent(p.first, dataMap);
			}

			output.clear();
			output << "Event Loop End"
				<< "\n****************************************************************************";
			StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
		}
	}
	catch (std::exception & e) {
		std::cout << e.what() << std::endl;
		std::stringstream output{};
		output << "Unexpected exception:\n" 
			<< e.what()
			<< "\nin Flightloop.";
		StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
		exit(1);
	}

	output.clear();
	output << "Fligh Loop end"
		<< "****************************************************************************************************\n";
	StreamLogger::log("XplaneHapticInterface", "liuHapticLog.txt", output);
	// Return 1.0 to indicate that we want this function to be called again in 1 second.
	return 1.0;
}

PLUGIN_API void	XPluginStop(void)
{
	// Destroy allocated resources.
	delete eventHandler;
	eventHandler = nullptr;
	XPLMUnregisterFlightLoopCallback(HapticFlightLoopCallback, NULL);
}

PLUGIN_API void XPluginDisable(void) {}

PLUGIN_API int XPluginEnable(void) { return 1; }

PLUGIN_API void XPluginReceiveMessage(
	XPLMPluginID	inFromWho,
	int				inMessage,
	void* inParam)
{}