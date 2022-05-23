#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <string>
#include "pyhelper.hpp"
#include "XPLMDataAccess.h"

//#define DEBUG
//#define TIME_CHECK


using DataRefMap = std::unordered_map<std::string, XPLMDataRef>;
using RefTypePair = std::pair<std::string, std::string>;
using EventToRefsMap = std::unordered_map<std::string, std::vector<RefTypePair>>;
using EventToFileVec = std::vector<std::string>;
using EventNameMap = std::unordered_map <std::string, int>;
using EventIndexPair = std::pair<std::string, int>;
using EventUsedVec = std::vector<bool>;
using TactFileNamesVec = std::vector<std::string>;
using TactFileVec = std::vector<std::string>;
using EventFileMap = std::unordered_map<std::string, std::string>;
using RefPathVector = std::vector<std::string>;
using PyFileNameVec = std::vector<std::string>;

