#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <string>
#include "pyhelper.hpp"
// Xplane specific include
#include "XPLMDataAccess.h"

//#define DEBUG
//#define TIME_CHECK

/**
 * @brief maps event name to its assosiated data reference in memory ( update to apropriate reference if used outside Xplane)
 * 
 */
using DataRefMap = std::unordered_map<std::string, XPLMDataRef>;

/**
 * @brief a pair linking the reference string of a datapoint to a string representation of its data type ( ie int, float or double)
 * 
 */
using RefTypePair = std::pair<std::string, std::string>;

/**
 * @brief maps event name to the assosiated vector of RefTypePairs
 * 
 */
using EventToRefsMap = std::unordered_map<std::string, std::vector<RefTypePair>>;

/**
 * @brief a vector of strings containing the tact file data associated to each event ( this uses the EventNameMap to retrive the correct index)
 * 
 */
using EventToFileVec = std::vector<std::string>;

/**
 * @brief Maps EventNames to its associated inted in the following vectors: EventToFileVec, EventUsedVec, TactFileNamesVec and PyFileNameVec
 * 
 */
using EventNameMap = std::unordered_map <std::string, int>;
using EventIndexPair = std::pair<std::string, int>;

/**
 * @brief a vector of booleans denoting if a event is currently used ( this uses the EventNameMap to retrive the correct index)
 * 
 */
using EventUsedVec = std::vector<bool>;

/**
 * @brief a vector of filenames for tact files associated with each event ( this uses the EventNameMap to retrive the correct index)
 * 
 */
using TactFileNamesVec = std::vector<std::string>;

/**
 * @brief vector of read files each index corresponds to a sertain event ( this uses the EventNameMap to retrive the correct index)
 * 
 */
using TactFileVec = std::vector<std::string>;

/**
 * @brief maps the event name to its associated tact file data
 * 
 */
using EventFileMap = std::unordered_map<std::string, std::string>;

/**
 * @brief a vector of all used datareference strings ( XPlane specific)
 * 
 */
using RefPathVector = std::vector<std::string>;

/**
 * @brief vector of all used pythonfiles 
 * 
 */
using PyFileNameVec = std::vector<std::string>;

