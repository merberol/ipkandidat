#pragma once

#include <iostream> 
#include "bHapticSDK/include/shared/HapticLibrary.h"
#include "pyhelper.hpp"
#include <unordered_map>
#define DEBUG


using EventToFileMap = std::unordered_map<std::string, std::string>;

class HapticInterface {

	EventToFileMap* eventMap{};
public:
	HapticInterface(std::string id) {
		#ifdef DEBUG
			AllocConsole();
			auto out = freopen("CONOUT$", "w", stdout);
			auto err = freopen("CONOUT$", "w", stderr);
			std::cout << "DEBUG: Active" << std::endl;
		#endif // DEBUG

		std::cout << "creating worker!" << std::endl;
	}

	~HapticInterface() {
		std::cout << "destroying worker" << std::endl;
	}

	void addFileMap(EventToFileMap* eventFileMap) {
		std::cout << "adding eventFileMap" << std::endl;
		eventMap = eventFileMap;
	}

	void Ready(){
#ifdef DEBUG
		std::cout << "running 'ReadyEvent'" << std::endl;
#endif // DEBUG
		RegisterFeedbackFromTactFile("ReadyEvent", eventMap->at("ReadyEvent").c_str());
		SubmitRegistered("ReadyEvent");
	}

	void SayHello() {
#ifdef DEBUG
		std::cout << "HapticInterface says Hello!" << std::endl;
#endif // DEBUG
	}

	void HighAltGearDown() {
#ifdef DEBUG
		//std::cout << "high alt, gear down warning!" << std::endl;
#endif // DEBUG
	}

	void LowAltGearUp() {
#ifdef DEBUG
		//std::cout << "low alt, gear up warning!" << std::endl;
#endif // DEBUG
	}

	void Stalling() {
#ifdef DEBUG
		std::cout << "stall warning!" << std::endl;
#endif // DEBUG
	}

	void Redout() {
#ifdef DEBUG
		std::cout << "redout warning!" << std::endl;
#endif // DEBUG
	}

	void Blackout() {
#ifdef DEBUG
		std::cout << "blackout warning!" << std::endl;
#endif // DEBUG
	}

};