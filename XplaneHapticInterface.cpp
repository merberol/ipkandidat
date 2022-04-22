// Downloaded from https://developer.x-plane.com/code-sample/timedprocessing/
// cl /EHsc /std:c++17 /Fe:"C:\X-Plane 11\Resources\plugins\LiuHaptics\win.xpl" /I includes\Xplane\CHeaders\XPLM /I includes\Python39 /I includes\bHaptics /I src XplaneHapticInterface.cpp /LD /INCREMENTAL:NO /link /LIBPATH:"C:\XplHaptInterface\libs"
#pragma comment(lib,"python39_d.lib")
#pragma comment(lib,"Xplane\\XPLM_64.lib")
#pragma comment(lib,"bHapticSDK\\bin\\win64\\haptic_library.lib")

/*
 * TimedProcessing.c
 * 
 * This example plugin demonstrates how to use the timed processing callbacks
 * to continuously record sim data to disk.
 * 
 * This technique can be used to record data to disk or to the network.  Unlike
 * UDP data output, we can increase our frequency to capture data every single
 * sim frame.  (This example records once per second.)
 * 
 * Use the timed processing APIs to do any periodic or asynchronous action in
 * your plugin.
 * 
 */

#if APL
#if defined(__MACH__)
#include <Carbon/Carbon.h>
#endif
#endif

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
static std::vector<float> prevPos{0,0,0};
static std::vector<float> currPos{0,0,0};
static bool highAlt = false;



EventHandler* eventHandler;
/* File to write data to. */
FILE*	gOutputFile;

/* Data refs we will record. */
static XPLMDataRef		gPlaneLat;
static XPLMDataRef		gPlaneLon;
static XPLMDataRef		gPlaneEl;

#if APL && __MACH__
static int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen);
#endif


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
#ifdef DEBUG
	//gOutputFile = fopen("liuHapticLog.txt", "w");// "w" means that we are going to write on this file

#endif
	eventHandler = new EventHandler("se.liu.haptic_plugin");

	if (remove("liuHapticLog.txt") != 0) {
		std::cout << "log not found" << std::endl;
	}
	else {
		std::cout << "log deleted" << std::endl;
	}

	std::ofstream outfile;
	std::cout << "eventhandler init" << std::endl;
	outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n*************** Building dataRefMap *************\n";

	// dataref strings required, get from config loader via EventHandlers interface
	for (int i = 0; i < eventHandler->refPathVec.size(); i++) {
		XPLMDataRef value = XPLMFindDataRef(eventHandler->refPathVec[i].c_str());
		outfile << "Emplacing key: " << eventHandler->refPathVec[i] << "with value: " <<  value <<   "\n";
		dataRefMap.emplace(eventHandler->refPathVec[i], value);
	}
	outfile << "\n*************** Building dataRefMap Done *************\n";
	outfile.close();

	std::cout << "datarefmap built" << std::endl;

	XPLMRegisterFlightLoopCallback(
		HapticFlightLoopCallback,	/* Callback */
		1.0,						/* Interval */
		NULL);						/* refcon not used. */
	// eventHandler->ReadyEvent();
	//fprintf(gOutputFile, "xplugin started\n");
	return 1;
}

float	HapticFlightLoopCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
                                   int                  inCounter,    
                                   void *               inRefcon)
{
	 	std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\nflight loop\n*************\n";
		outfile.close();
	try {

		// p is a pair of Event Name and a integer representing the index of the event.
		for (std::pair<std::string, int> p : eventHandler->eventNameMap) {
			if (eventHandler->getIsUsed(p.first)) {
	#ifdef DEBUG
					 	std::ofstream outfile;
		outfile.open("liuHapticLog.txt", std::ios_base::app);
  		outfile << "\n***************\n Running event " << p.first << "\n*************\n";
		outfile.close();
	#endif
				eventHandler->RunEvent(p.first, dataRefMap);
			}
		}
	}
	catch (std::exception & e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}

	/* Return 1.0 to indicate that we want to be called again in 1 second. */
	return 1.0;

}

PLUGIN_API void	XPluginStop(void)
{
	// Call destructor for allocated resources
	//fclose(gOutputFile);
	delete eventHandler;
	eventHandler = nullptr;
	XPLMUnregisterFlightLoopCallback(HapticFlightLoopCallback, NULL);
}

PLUGIN_API void XPluginDisable(void) {}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
	XPLMPluginID	inFromWho,
	int				inMessage,
	void* inParam)
{}

#if APL && __MACH__
#include <Carbon/Carbon.h>
int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen)
{
	CFStringRef inStr = CFStringCreateWithCString(kCFAllocatorDefault, inPath ,kCFStringEncodingMacRoman);
	if (inStr == NULL)
		return -1;
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLHFSPathStyle,0);
	CFStringRef outStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	if (!CFStringGetCString(outStr, outPath, outPathMaxLen, kCFURLPOSIXPathStyle))
		return -1;
	CFRelease(outStr);
	CFRelease(url);
	CFRelease(inStr); 	
	return 0;
}
#endif

