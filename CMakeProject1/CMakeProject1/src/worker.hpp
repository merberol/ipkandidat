#pragma once

#ifndef DEBUG
#define DEBUG 1
#endif // !DEBUG

class Worker {

public:
	Worker() {
		std::cout << "instansiating Worker" << std::endl;
		
	}

	~Worker() {
		std::cout << "destrying worker" << std::endl;
	}

	void sayHello() {
		std::cout << "Worker says Hello" << std::endl;
	}


};
