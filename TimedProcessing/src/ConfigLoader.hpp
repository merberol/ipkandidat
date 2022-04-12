#pragma once 

#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include "EventHandler.hpp"
#include "bHapticSDK/include/shared/HapticLibrary.h"

using resultType = std::pair<std::unordered_map<std::string, bool>, std::unordered_map<std::string, std::string>>;


struct ConfigLoader {


public:

	ConfigLoader() {

	}

	void SayHello() {
		std::cout << "ConfigLoader says Hello!" << std::endl;
	}

	resultType run() {
		// Read from file
		std::string fileData = "";
		
		// Parse file to csv
		std::ofstream config;
		//"EventName;TactFileName;Used;MiscAllowed;Misc";	// Excel readable header line

		// Create map to return
		resultType data = loadData();
		std::cout << "exiting run" << std::endl;
		return data;
	}

	resultType loadData() {
		std::vector < std::pair<std::string, std::string>> FiletoEventName{};
		std::unordered_map<std::string, bool> dataMap;
		std::unordered_map<std::string, std::string> eventFileMap;
		std::fstream fileIn;
		fileIn.open("Resources\\plugins\\LiuHaptics\\ConfigCSV.csv", std::fstream::in);
		if (fileIn.is_open()) {
			std::cout << "file is open" << std::endl;
		}
	
		std::string word{};
		std::vector<std::string> row{};
		bool first{ true };
		while (!fileIn.eof())
		{
			row.clear();
			std::string line{};
			// skip headers in CSV file 
			if (first) {
				std::getline(fileIn, line, '\n');
				line = "";
				first = !first;
			}
			std::getline(fileIn, line, '\n');
			std::stringstream ss;
			ss << line;

			while (std::getline(ss, word, ';') ) {
				row.push_back(word);
			}
			if (row.empty())
			{
				break;
			}
			// register event with Player

			std::string eventName = row[0];
			std::string tactfilename = row[1];

			FiletoEventName.push_back(std::pair<std::string, std::string>(tactfilename, eventName));
			

			// create event map
			bool used = stoi(row[2]);
			dataMap.emplace(eventName, used);
		}
		fileIn.close();
		std::cout << "closed main data file" << std::endl;

		for (std::pair<std::string, std::string> p : FiletoEventName) {
			std::string file = LoadTactFile(p.first);
			std::cout << p.first << " is file name" << std::endl;
			eventFileMap.emplace(p.second,file);
		}
		std::cout << "returning datamap" << std::endl;
		resultType result{ dataMap, eventFileMap };
		return result;
	}
	
	std::string LoadTactFile(std::string fileName = "test.tact") {
		std::cout << "trying to open" << fileName << std::endl;
		// test.tact file should be in the same folder
		std::ifstream fileIn("Resources\\plugins\\LiuHaptics\\tactFiles\\" + fileName);
		if (fileIn.is_open()) {
			std::cout << fileName << " is open" << std::endl;
		}
		else {
			std::cout << ".tact file failed to open" << std::endl;
		}
		char* inputString = new char[100000];
		if (fileIn.good()) {
			std::cout << "file is good " << std::endl;
			while(!fileIn.eof()){
				fileIn.getline(inputString, 100000);
			}
			fileIn.close();
			std::cout << "file is read" << std::endl;
			return inputString;
		} else {
			if(fileIn.is_open())
				fileIn.close();
			std::cout << "Failed to register the feedback file." << std::endl;
			return "";
		}
	}

};