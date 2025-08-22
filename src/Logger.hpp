/**
 * @file Logger.hpp
 * @author Charlie Simonsson simonsson.charlie@gmail.com & Marcus Franzén
 * @brief 
 * @version 1
 * @date 2022-05-23
 * 
 * @copyright Copyright (c) 2022
 * 
 * Authors: 
 * Licence
 * The MIT License (MIT)
 *
 * Copyright (c) <2022> <Charlie simonsson & Marcus Franzén>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *~
 * ~~~~~~~~ Change LOG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *~
 * ~~ Add and record of any changes and bug fixes to the system in this section
 * ~~ of the file where those changes where made.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *~
 * may 23 2022: Added Licence and change log : Charlie
 * 
 */
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdarg>
#include <ctime>

/**
 * @brief A simple logger designed to write log to a log file implemeted as a static class
 * 
 * -- 
 * 
 * -- methods
 * static void log(std::string const& context, std::string const& fileName, std::string const& message)
 * static void log(std::string const& context, std::string const& fileName, std::stringstream const& message)
 * static void lograw(std::string const& fileName,  std::string const& message)
 * 
 */
class StreamLogger {
 
public:
   /**
    * @brief writes a log to the indicated filename by appending to the file. 
    * 
    * @param context a string detailing the origin of the log
    * @param fileName the file to write the log to
    * @param message a std::string with the message / info to be logged
    */
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
 
    /**
     * @brief writes a log to the indicated filename by appending to the file. 
     * 
     * @param context a string detailing the origin of the log
     * @param fileName the file to write the log to
     * @param message a std::stringstream with the message / info to be logged
     */
    static void log(std::string const& context, std::string const& fileName, std::stringstream const& message){
        StreamLogger::log(context, fileName, message.str());
        return;
    }

    /**
     * @brief logs a string to the indicated file without any formating.
     *  use to log various text data with a predetermined formatting for instance a pregenerated csv file
     * 
     * @param fileName the file to write the log to
     * @param message a std::string with the data to be logged
     */
    static void lograw(std::string const& fileName,  std::string const& message){
        std::ofstream ofile;
        ofile.open(fileName, std::ios_base::app);
        ofile << message;
        ofile.close();
        return;
    }
    
};