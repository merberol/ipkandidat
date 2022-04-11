#pragma once



#include <windows.h>
#include "worker.hpp"
#include "ConfigLoader.hpp"
#include "bHapticSDK/include/shared/HapticLibrary.h"
#include <chrono>

using resultType = std::pair<std::unordered_map<std::string, bool>, std::unordered_map<std::string, std::string>>;


class EventHandler
{

public:
	std::string _id;
	std::unordered_map<std::string, bool> eventMap;
	std::unordered_map<std::string, std::string> eventFileMap;
	
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
		resultType result = configLoader.run();
		eventMap = result.first;
		eventFileMap = result.second;
		worker.addFileMap(&eventFileMap);

	};

	~EventHandler() {

	}

	void ReadyEvent() {
		std::cout << "receiving ready event from xplane and forwarding to worker" << std::endl;
		worker.Ready();
	}

	bool DoGearEvent() {
		
		std::cout << "checking eventMap" << std::endl;;
		bool result{};
		try {
			result = eventMap.at("GearUppWarn") || eventMap.at("GearDownWarn");
		}
		catch (std::exception& e) {
			result = false;
		}
		return result;
	}

	void GearEvent(double speed, int elevation, float gearDeployment) {
		
		if (elevation >= 1000 && gearDeployment < 1.0 ){
			std::cout << "receiving gear event from xplane and forwarding HighAlt to worker" << std::endl;
			worker.HighAlt();
		}
		if (elevation < 500 && gearDeployment > 0.0) {
			std::cout << "receiving gear event from xplane and forwarding LowAlt to worker" << std::endl;
			worker.LowAlt();
		}
	}

	bool DoStallingEvent() {
		std::cout << "checking if stalling event active" << std::endl;
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
		std::cout << "checking if redout and blackout events are active" << std::endl;
		return true;
	}

	void RedoutEvent(int redout) {
		if (redout == 1) {
			std::cout << "receiving redout event from xplane and forwarding redout to worker" << std::endl;
			worker.Redout();
		}
	}

	void BlackoutEvent(int blackout) {
		if (blackout == 1) {
			std::cout << "receiving blackout event from xplane and forwarding black to worker" << std::endl;
			worker.Blackout();
		}
	}

private:
	Worker worker{_id};

	


};

