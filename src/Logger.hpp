#include <iostream>
#include <fstream>
#include <cstdarg>

class StreamLogger {
    std::ofstream outfile;
public:
    void slog (int numArgs, ...) {
        va_list argList;
        outfile.open("liuHapticLog.txt", std::ios_base::app);
        va_start(argList, numArgs);
        for (int i = 0; i < numArgs; i++) {
            outfile << va_arg(argList, int);
        }
        outfile << std::endl;
		outfile.close();
        va_end(argList);
        return;
    }
};