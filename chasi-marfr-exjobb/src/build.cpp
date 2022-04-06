// for operating systems:       g++ build.cpp -lstdc++ -std=c++17 -Og -o build.out
// for non-operating systems:   gcc build.cpp -std=c++17 -o build.exe
// for non-operating systems:   clang build.cpp -std=c++17 -o build.exe
#include <string>
#include <vector>

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;


/* CONFIG BEGIN */
#if defined(__clang__)
string default_compiler = "clang";
#elif defined(__GNUC__)
string default_compiler = "gcc";
#endif