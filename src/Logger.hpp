#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdarg>
#include <ctime>

class StreamLogger {
    std::ofstream outfile;
public:
  
   static void log(std::string const& context, std::string const& fileName, std::string const& message){
        std::ofstream ofile;
        ofile.open(fileName, std::ios_base::app);
        std::time_t result = std::time(nullptr);
        ofile << "****************************************\n" 
            << context << " | "
            << std::asctime(std::localtime(&result))
            << message 
            << "\n****************************************" << std::endl;
        
        ofile.close();
        return;

    }

    static void log(std::string const& context, std::string const& fileName, std::stringstream const& message){
        StreamLogger::log(context, fileName, message.str());
        return;
    }
    
};