// CMakeProject1.cpp : Defines the entry point for the application.
//

#include "CMakeProject1.h"
#include "src/worker.hpp"

using namespace std;
Worker worker{};
int main()
{
	cout << "Hello CMake." << endl;
	worker.sayHello();
	return 0;
}
