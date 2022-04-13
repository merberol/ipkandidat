#pragma once



#include <windows.h>
#include "HapticInterface.hpp"
#include "ConfigLoader.hpp"
#include "bHapticSDK/include/shared/HapticLibrary.h"
#include <chrono>
#include <tuple>
#include "pyFunction.hpp"

#include <unordered_map>



class EventHandler
{

public:
	std::string _id;
	std::unordered_map<std::string, bool> eventMap;
	std::unordered_map<std::string, std::string> eventFileMap;
	std::vector<std::string> dataRefVector;
	
	EventHandler() = default;

	EventHandler(std::string id) 
		:_id{ id }
	{
		char playerPath[200];
		int size = 2;
		bool res = TryGetExePath(playerPath, size);
		auto path = GetExePath();

		if (res) {
			std::cout << "1. getExePath bHaptics Player is installed:  " << playerPath << std::endl << path << std::endl;
			std::cout << "1. player path size:  " << size << std::endl;

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

		worker.SayHello();
		ConfigLoader configLoader{};
		configLoader.SayHello();
		ResultType result = configLoader.run();
		eventMap = std::get<0>(result);
		eventFileMap = std::get<1>(result);
		dataRefVector = std::get<2>(result);
		worker.addFileMap(&eventFileMap);
	};

	~EventHandler() {

	}

	void ReadyEvent() {
		//std::cout << "receiving ready event from xplane and forwarding to worker" << std::endl;
		worker.Ready();
	}

	bool DoGearEvent() {
		//std::cout << "checking eventMap" << std::endl;;
		bool result{};
		try {
			result = eventMap.at("GearUpWarn") || eventMap.at("GearDownWarn");
		}
		catch (std::exception& e) {
			result = false;
		}
		return result;
	}

	void GearEvent(double speed, int elevation, float gearDeployment) {
#ifdef DEBUG
		//std::cout << "entering gear event with gear deployment: " << gearDeployment << std::endl;
#endif //DEBUG
		if (elevation >= 1000 && gearDeployment > 0.0 ){
			worker.HighAltGearDown();
		}
		if (elevation < 500 && gearDeployment < 1.0) {
			worker.LowAltGearUp();
		}
	}

	bool DoStallingEvent() {
		//std::cout << "checking if stalling event active" << std::endl;
		bool result{};
		try {
			result = eventMap.at("StallWarn");
		}
		catch (std::exception& e) {
			result = false;
		}
		return result;
	}

	void StallingEvent(int stallWarning) {
		if (stallWarning == 1) {
			std::cout << "receiving stall event from xplane and forwarding Stalling to worker" << std::endl;
			worker.Stalling();
		}
	}

	bool DoXOutEvents() {
		//std::cout << "checking if redout and blackout events are active" << std::endl;
		return false;
	}

	void RedoutEvent(int redout) {
		if (redout == 1) {
			//std::cout << "receiving redout event from xplane and forwarding redout to worker" << std::endl;
			worker.Redout();
		}
	}

	void BlackoutEvent(int blackout) {
		if (blackout == 1) {
			//std::cout << "receiving blackout event from xplane and forwarding black to worker" << std::endl;
			worker.Blackout();
		}
	}

private:
	HapticInterface worker{_id};

	


};

