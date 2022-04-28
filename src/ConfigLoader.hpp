#pragma once 

#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <algorithm>
#include "includes\bHaptics\HapticLibrary.h"
#include "Logger.hpp"



struct ConfigLoader {

	ConfigLoader() {
#ifdef DEBUG
		StreamLogger::log("ConfigLoader : Constructor", "liuHapticLog.txt",  "### Creating ConfigLoader ###" );
#endif
	}

	void run(EventNameMap & eventNameMap, EventUsedVec & eventUsed,  TactFileVec & tactFiles, RefPathVector & refPathVec,
			std::vector<std::vector<RefTypePair>> & eventTypeRefs,  PyFileNameVec & pyFileNames ) {

		TactFileNamesVec tactFileNames{};

		processConfig(
		eventNameMap,
		eventUsed,
		tactFileNames,
		eventTypeRefs,
		pyFileNames,
		refPathVec
		);

		if ( eventUsed.size() != tactFileNames.size() || tactFileNames.size() != pyFileNames.size() || pyFileNames.size() != eventTypeRefs.size() )
		{
			StreamLogger::log("ConfigLoader : run", "liuHapticLog.txt", "Vector sizes don't match!");
			exit(1);
		}

		for (std::string fileName : tactFileNames) {
			std::string file = loadTactFile(fileName);
			tactFiles.push_back(file);
		}

		return;
	}
	
	std::string loadTactFile(std::string fileName = "test.tact") {
#ifdef DEBUG
		{
			std::stringstream output{};
			output << "trying to open" << fileName;
			StreamLogger::log("EventHandler : runEvent", "liuHapticLog.txt", output);
		}
#endif
		// test.tact file should be in the same folder
		std::ifstream fileIn("Resources\\plugins\\LiuHaptics\\tactFiles\\" + fileName);
		if (!fileIn.is_open()) {
			StreamLogger::log("ConfigLoader : loadTactFile", "liuHapticLog.txt", fileName + " failed to open");
			exit(1);	
		}

#ifdef DEBUG
		StreamLogger::log("EventHandler : runEvent", "liuHapticLog.txt", fileName + " is open");
#endif
	
		char* inputString = new char[100000];
		if (fileIn.good()) {
			while(!fileIn.eof()){
				fileIn.getline(inputString, 100000);
			}
			fileIn.close();
			return inputString;
		} else {
			if(fileIn.is_open())
				fileIn.close();
			StreamLogger::log("ConfigLoader : loadTactFile", "liuHapticLog.txt", "Failed to load the feedback file due to file error.");
			return "";
		}
	}

	void processConfig(
		std::unordered_map<std::string, int> &eventNameMap,
		std::vector<bool> &eventUsed,
		std::vector<std::string> &tactFileNames,
		std::vector<std::vector<RefTypePair>> &eventTypeRefs,
		std::vector<std::string> &pyFileNames,
		std::vector<std::string> &refVec
		){
		std::fstream fileIn;
		fileIn.open("Resources\\plugins\\LiuHaptics\\ConfigCSV.csv", std::fstream::in);
		if (!fileIn.is_open()) {
			StreamLogger::log("ConfigLoader : processConfig", "liuHapticLog.txt", "Failed to open ConfigCSV.csv.");
			exit(1);
		}

		std::vector<std::string> row{};
		bool first{ true };
		int index = 0;

		while (!fileIn.eof())
		{
			if (row.empty()) {
				break;
			}
			else {
				fillRowWithConfigElements(row, first, fileIn);
			}
		
			std::vector<RefTypePair> eventRefs{};
			std::string eventName = row[0];
			std::string tactfilename = row[1];
			bool used = stoi(row[2]);
			std::string pyFileName = row[3];
			pyFileNames.push_back(pyFileName);
			int numDataPoints = stoi(row[4]);
	
			fillDataRefVectors(numDataPoints, row, refVec, eventRefs);
			row.clear();

			eventTypeRefs.push_back(eventRefs);
			tactFileNames.push_back(tactfilename);
			eventUsed.push_back(used);
			eventNameMap.emplace(eventName, index);
			index++;
		}
		fileIn.close();
		removeDuplicateRefs(refVec);
	}

	void fillRowWithConfigElements(std::vector<std::string> & row, bool & first, std::fstream & fileIn) {
		std::string line{};
		std::string word{};
		// Skip headers in CSV file
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
	}

	void fillDataRefVectors(int const& numDataPoints, std::vector<std::string> const& row, std::vector<std::string> & refVec, std::vector<RefTypePair> & eventRefs) {
		if (numDataPoints > 0) {
			for ( int i = 0; i < numDataPoints * 2; i+=2){
				int idx = i+5;
				refVec.push_back(row[idx+1]);
				RefTypePair tmp{row[idx], row[idx+1]};
				eventRefs.push_back(tmp);
			}
		}
	}

	void removeDuplicateRefs(std::vector<std::string> & refVec) {
		std::sort(begin(refVec), end(refVec));
		auto c_itr = std::unique(begin(refVec), end(refVec));
		refVec.erase(c_itr, refVec.end());
	}
};