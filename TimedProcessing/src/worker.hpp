#pragma once

#include <iostream> 

 //#include <glog/logging.h>

#define DEBUG

class Worker {
public:

	Worker() {
#ifdef DEBUG
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
#endif // DEBUG

		std::cout << "creating worker!" << std::endl;
	}

	~Worker() {
		std::cout << "destroying worker" << std::endl;
	}

	void SayHello() {
		std::cout << "worker says Hello!" << std::endl;
	}

	void HighAlt() {
		std::cout << "gear down warning!" << std::endl;
	}

};