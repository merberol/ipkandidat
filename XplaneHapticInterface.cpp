
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
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"


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

	if (remove("liuHapticLog.txt") != 0) {
		std::cout << "Log not found: Unable to delete." << std::endl;
	}
	else {
		std::cout << "Log found: Deleted." << std::endl;
	}

	std::ofstream outfile;
	outfile.open("liuHapticLog.txt", std::ios_base::app);
  	outfile << "\n***************\nBuilding dataRefMap\n***************\n";

	// dataref strings required, get from config loader via EventHandlers interface
	for (int i = 0; i < eventHandler->refPathVec.size(); i++) {
		XPLMDataRef value = XPLMFindDataRef(eventHandler->refPathVec[i].c_str());
		outfile << "Emplacing key: " << eventHandler->refPathVec[i] << " with value: " <<  value <<   "\n";
		dataRefMap.emplace(eventHandler->refPathVec[i], value);
	}
	outfile.close();

	XPLMRegisterFlightLoopCallback(
		HapticFlightLoopCallback,	/* Callback */
		1.0,						/* Interval */
		NULL);						/* refcon not used. */

	return 1;
}

std::unordered_map<std::string, double> getData(std::string eventName) {
	std::vector<RefTypePair> dataRefStrings = eventHandler->eventTypeRefs[eventHandler->getIndex(eventName)];
	std::unordered_map<std::string, double> data{};
	int counter = 0;

	for (RefTypePair refTypePair : dataRefStrings) {
		XPLMDataRef dataRef = dataRefMap.at(refTypePair.second);
		std::string valType = refTypePair.first;

		if (valType == "int") {
			long dataValue = XPLMGetDatai(dataRef);
			std::cout << valType << " " << refTypePair.first << ": " << dataValue << std::endl;
			data.emplace(refTypePair.second, dataValue);
		}
		else if (valType == "float") {
			float dataValue = XPLMGetDataf(dataRef);
			std::cout << valType << " " << refTypePair.first << ": " << dataValue << std::endl;
			data.emplace(refTypePair.second, dataValue);
		}
		else if (valType == "double") {
			double dataValue = XPLMGetDatad(dataRef);
			std::cout << valType << " " << refTypePair.first << ": " << dataValue << std::endl;
			data.emplace(refTypePair.second, dataValue);
		}
		else {
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
			outfile << "Unusable value type in config: " << valType << "\n";
			outfile << "Use only int, float or double.\n";
			outfile.close();
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
	try {
		for (EventIndexPair p : eventHandler->eventNameMap) {
			if (eventHandler->getIsUsed(p.first)) {
				dataMap = getData(p.first);
				eventHandler->runEvent(p.first, dataMap);
			}
		}
	}
	catch (std::exception & e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}

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