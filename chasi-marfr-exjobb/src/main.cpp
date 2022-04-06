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
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <math.h>

#include "src/worker.hpp"

#define MAX_REFS 4
static char DataRefString[MAX_REFS][255] = { "sim/flightmodel/position/elevation", "sim/flightmodel/position/local_x", "sim/flightmodel/position/local_y", "sim/flightmodel/position/local_z" };
static XPLMDataRef storedDataRefs[MAX_REFS];
static std::unordered_map<std::string, XPLMDataRef> dataRefMap{};
static std::vector<float> prev_pos{};
static std::vector<float> curr_pos{};
static bool high_alt = false;

static int GetElevation(void);
static std::vector<float> GetPosition(void);
static double CalculateDistance(std::vector<float> pos1, std::vector<float> pos2);
static double CalculateSpeed(float);
static int DetectRedout(void);
static int DetectBlackout(void);

Worker worker{};

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
	google::InitGoogleLogging("Xplane Haptic plugin");
	LOG(INFO) << "Starting Plugin";
	strcpy_s(outName, 18, "LIU Haptic Plugin");
	strcpy_s(outSig, 18, "liu.haptic_plugin");
	strcpy_s(outDesc, 55, "A plugin for testing a haptic vest with the flightsim.");
	
	worker.sayHello();
	
	LOG(INFO) << " building data ref map ";
	for (int dataRef = 0; dataRef < MAX_REFS; dataRef++) {
		dataRefMap.emplace(DataRefString[dataRef], XPLMFindDataRef(DataRefString[dataRef]));
		storedDataRefs[dataRef] = XPLMFindDataRef(DataRefString[dataRef]);
	}
	LOG(INFO) << " done building data ref map ";
	// Flight loop for calculating time?
	XPLMRegisterFlightLoopCallback(
		HapticFlightLoopCallback,	/* Callback */
		1.0,						/* Interval */
		NULL);						/* refcon not used. */

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	LOG(INFO) << "stopping plugin";
	// Call destructor for allocated resources
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
	try {
		double speed = CalculateSpeed(inElapsedSinceLastCall);
		int elevation = GetElevation();
		high_alt = elevation >= 1000;
		if (high_alt) { worker.highAlt(); }

	}
	catch (std::exception &e) {
		LOG(FATAL) << e.what();
	}
	
	
	/* Return 1.0 to indicate that we want to be called again in 1 second. */
	return 1.0;
}

int GetElevation(void)
{
	int elevation{ -1 };
	try {
		int elevation = XPLMGetDatai(
			dataRefMap.at("sim/flightmodel/position/elevation")
		);
	}
	catch (std::exception& e) {
		LOG(FATAL) << e.what();
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
		LOG(FATAL) << e.what();
	}
	return pos_vector;
}



/*
* runs once every second
*/
double CalculateSpeed(float delta)
{
	double speed;
	try {
		prev_pos = curr_pos;
		curr_pos = GetPosition();
		speed = CalculateDistance(curr_pos, prev_pos) / delta;
	}
	catch (std::exception& e) {
		LOG(FATAL) << e.what();
	}
	return speed;
}    

double CalculateDistance(std::vector<float> pos1, std::vector<float> pos2)
{
	double distance{};
	try {

		distance = sqrt(
			pow(pos2[1] - pos1[1], 2) +
			pow(pos2[2] - pos1[2], 2) +
			pow(pos2[3] - pos1[3], 2)
		);
	}
	catch (std::exception& e)
	{
		LOG(FATAL) << e.what();
	}
	return distance;
}

int DetectRedout(void)
{
	
}

int DetectBlackout(void)
{

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

