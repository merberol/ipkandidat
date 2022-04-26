#pragma once 

#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <algorithm>
#include "includes\bHaptics\HapticLibrary.h"
#include "pyFunction.hpp"


struct ConfigLoader {

	ConfigLoader() {
		std::cout << "### Creating ConfigLoader ###" << std::endl;
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
			std::ofstream outfile;
			outfile.open("liuHapticLog.txt", std::ios_base::app);
  			outfile << "Vector sizes don't match!";
			outfile << eventUsed.size() << " / " << tactFileNames.size() << " / " << pyFileNames.size() << " / " << eventTypeRefs.size();
			outfile.close();
			exit(1);
		}

		for (std::string fileName : tactFileNames) {
			std::string file = loadTactFile(fileName);
			tactFiles.push_back(file);
		}

		return;
	}
	
	std::string loadTactFile(std::string fileName = "test.tact") {
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
		if (fileIn.is_open()) {
			std::cout << "file is open" << std::endl;
		}
		std::string word{};
		std::vector<std::string> row{};
		bool first{ true };
		int index = 0;
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
			std::vector<RefTypePair> eventRefs{};
			std::string eventName = row[0];
			std::string tactfilename = row[1];
			bool used = stoi(row[2]);
			std::string pyFileName = row[3];
			pyFileNames.push_back(pyFileName);
			int numDataPoints = stoi(row[4]);
			
			std::cout << "\nprocessConfig for " << eventName << " with " << numDataPoints << " data points" << std::endl;
			
			if (numDataPoints > 0) {
				for ( int i = 0; i < numDataPoints * 2; i+=2){
					int idx = i+5;
					std::cout << "type : " << row[idx] << std::endl;
					std::cout << "refstring : " << row[idx+1] << std::endl;
					refVec.push_back(row[idx+1]);
					RefTypePair tmp{row[idx], row[idx+1]};
					eventRefs.push_back(tmp);
				}
			}
			eventTypeRefs.push_back(eventRefs);
			std::cout << "pushing " << tactfilename << " to tactFiles" << std::endl;
			tactFileNames.push_back(tactfilename); // tact file to event
			std::cout << "pushing " << used << " to eventUsed" << std::endl;
			eventUsed.push_back(used);
			std::cout << "emplacing " << eventName << " and " << index << " to eventNameMap" << std::endl;
			eventNameMap.emplace(eventName, index);
			index++;
		}
		fileIn.close();

		// Remove duplicates from refVec.
		std::sort(begin(refVec), end(refVec));
		auto c_itr = std::unique(begin(refVec), end(refVec));
		refVec.erase(c_itr, refVec.end());
	}

};