#pragma once

#include <iostream> 

 //#include <glog/logging.h>

class Worker {
public:

	Worker() {
		
		std::cout << "creating worker!" << std::endl;
	}

	~Worker() {
		//LOG(INFO) << "destroying worker";
	}

	void sayHello() {
		std::cout << "worker says Hello!" << std::endl;
	}

	void highAlt() {
		std::cout << "gear down warning!" << std::endl;
	}

};