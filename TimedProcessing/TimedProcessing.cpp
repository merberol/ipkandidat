// Downloaded from https://developer.x-plane.com/code-sample/timedprocessing/



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
#include <stdio.h>
#include <string.h>
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <math.h>
#include "src/EventHandler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

static std::vector<std::string> DataRefString{ "sim/flightmodel/position/elevation", "sim/flightmodel/position/local_x", "sim/flightmodel/position/local_y", "sim/flightmodel/position/local_z", "sim/flightmodel/failures/stallwarning", "sim/aircraft/gear/acf_gear_retract", "sim/aircraft/parts/acf_gear_deploy"};
static std::unordered_map<std::string, XPLMDataRef> dataRefMap{};
static std::vector<float> prevPos{0,0,0};
static std::vector<float> currPos{0,0,0};
static bool highAlt = false;

static int GetElevation(void);
static std::vector<float> GetPosition(void);
static float GetGearDeployed(void);
static int GetStallWarning(void);
static double CalculateDistance(std::vector<float> pos1, std::vector<float> pos2);
static double CalculateSpeed(float);
static int DetectRedout(void);
static int DetectBlackout(void);
int GetGearRetractable(void);





EventHandler* eventHandler;
/* File to write data to. */
static FILE *	gOutputFile;

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
	
	eventHandler = new EventHandler{ "se.liu.haptic_plugin" };

	for (int i = 0; i < DataRefString.size(); i++) {
		dataRefMap.emplace(DataRefString[i], XPLMFindDataRef(DataRefString[i].c_str()));
	}

	XPLMRegisterFlightLoopCallback(
		HapticFlightLoopCallback,	/* Callback */
		1.0,						/* Interval */
		NULL);						/* refcon not used. */
	// eventHandler->ReadyEvent();
	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	// Call destructor for allocated resources
	delete eventHandler;
	eventHandler = nullptr;


	XPLMUnregisterFlightLoopCallback(HapticFlightLoopCallback, NULL);
}

PLUGIN_API void XPluginDisable(void)
{
	// Stop communicating with worker

}

PLUGIN_API int XPluginEnable(void)
{
	// Start communicating with worker
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					int				inMessage,
					void *			inParam)
{
}

bool first{ true };

float	HapticFlightLoopCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
                                   int                  inCounter,    
                                   void *               inRefcon)
{
	/* The actual callback.  First we read the sim's time and the data. */
	float	elapsed = XPLMGetElapsedTime();
	float	lat = XPLMGetDataf(gPlaneLat);
	float	lon = XPLMGetDataf(gPlaneLon);
	float	el = XPLMGetDataf(gPlaneEl);

	if (first) {
#ifdef DEBUG
		std::cout << "sending ready event from Xplane interface" << std::endl;
#endif // DEBUG
		eventHandler->ReadyEvent();
		first = !first;
	}
	
	if (eventHandler->DoGearEvent() && GetGearRetractable()) {
#ifdef DEBUG
		std::cout << "sending gear event from Xplane interface" << std::endl;
#endif // DEBUG

		eventHandler->GearEvent(
			CalculateSpeed(inElapsedSinceLastCall),
			GetElevation(),
			GetGearDeployed()
		);
	}

	if (eventHandler->DoStallingEvent() && XPLMGetDatai(dataRefMap.at("sim/flightmodel/failures/stallwarning"))) {
#ifdef DEBUG
		std::cout << "sending stall event from Xplane interface" << std::endl;
#endif // DEBUG
		eventHandler->StallingEvent(
			GetStallWarning()
		);
	}

	if (eventHandler->DoXOutEvents()) {
#ifdef DEBUG
		std::cout << "sending readout and blackout events from Xplane interface" << std::endl;
#endif // DEBUG
		eventHandler->RedoutEvent(DetectRedout());
		eventHandler->BlackoutEvent(DetectBlackout());
	}
	
	/* Return 1.0 to indicate that we want to be called again in 1 second. */
	return 1.0;
}

int GetElevation(void)
{
	int elevation{ -1 };
	try {
		elevation = XPLMGetDatai(
			dataRefMap.at("sim/flightmodel/position/elevation")
		);
	}
	catch (std::exception& e) {
		
	}
	return elevation;
}

std::vector<float> GetPosition(void)
{	
	std::vector<float> pos_vector{ 0,0,0 };
	try {
		pos_vector = {
			XPLMGetDataf(dataRefMap.at("sim/flightmodel/position/local_x")),
			XPLMGetDataf(dataRefMap.at("sim/flightmodel/position/local_y")),
			XPLMGetDataf(dataRefMap.at("sim/flightmodel/position/local_z"))
		};
	}
	catch (std::exception& e) {
		
	}
	return pos_vector;
}

/**
* @returns landing gear deployment, 0.0->1.0 
*/
float GetGearDeployed(void) {
	float deployed{ XPLMGetDataf(dataRefMap.at("sim/aircraft/parts/acf_gear_deploy")) };
	return deployed;
}

/**
* @returns if gear is retractable, 0->1
*/
int GetGearRetractable(void) {
	int retractable{ XPLMGetDatai(dataRefMap.at("sim/aircraft/gear/acf_gear_retract")) };
	return retractable;
}

/**
* @returns if plane is stalling, 0->1
*/
int GetStallWarning(void) {
	int stallWarning{ XPLMGetDatai(dataRefMap.at("sim/flightmodel/failures/stallwarning")) };
	return stallWarning;
}

/*
* runs once every second
*/
double CalculateSpeed(float delta)
{
	double speed;
	try {
		prevPos = currPos;
		currPos = GetPosition();
		speed = CalculateDistance(currPos, prevPos) / delta;
	}
	catch (std::exception& e) {
		
	}
	return speed;
}    

double CalculateDistance(std::vector<float> pos1, std::vector<float> pos2)
{
	double distance{};
	try {
		distance = sqrt(
			pow(pos2[0] - pos1[0], 2) +
			pow(pos2[1] - pos1[1], 2) +
			pow(pos2[2] - pos1[2], 2)
		);
	}
	catch (std::exception& e)
	{
		
	}
	return distance;
}

int DetectRedout(void)
{
	return 0;
}

int DetectBlackout(void)
{
	return 0;
}                               




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

