#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <string>
#include <stdio.h>
#include <memory>
#include "Python.h"
#include "pyhelper.hpp"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "pyFunction.hpp"

using PyFuncPtr = std::shared_ptr<PyFunc>;
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
using PyFuncVec = std::vector<PyFuncPtr>;
using ResultType = std::tuple<
    EventNameMap,
    EventUsedVec,
    TactFileVec,
    RefPathVector,
    std::vector<std::vector<RefTypePair>>,
    PyFileNameVec
>;