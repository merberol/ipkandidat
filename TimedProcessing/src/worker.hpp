#pragma once

#include <iostream> 

#include <glog/logging.h>

class Worker {
public:

	Worker() {
		

		LOG(INFO) << "instansiating Worker";
	}

	~Worker() {
		LOG(INFO) << "destroying worker";
	}

	void sayHello() {
		LOG(INFO) << "worker says Hello!";
	}

	void highAlt() {
		LOG(INFO) << "seeing gear down warning";
	}

};