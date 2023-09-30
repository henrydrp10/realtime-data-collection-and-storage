#ifndef JSONFUNCTIONS_H
#define JSONFUNCTIONS_H

#include <fstream>
#include <iostream>

#include <json/json.h>

#include "dataChannel.h"
#include "dataInput.h"
#include "dataPoint.h"
#include "extractedSubChannel.h"
#include "timer.h"

/**
 * @brief JSON Utility functions.
 * 
 * @details This set of functions will help us load one
 * or all channels from a specific or the general JSON 
 * file respectively. They will also help us save one or
 * all channels to a general or specific JSON file, 
 * respectively.
 */

void saveJson(const std::unordered_map<uint16_t, DataChannel>& channels);
void saveChannel(const DataChannel& channel);
void loadChannel(DataChannel& channelLoaded, const Json::Value& obj);
void loadJson(std::unordered_map<uint16_t, DataChannel>& channelsLoaded);
void loadJsonChannel(DataChannel& channelLoaded, uint16_t targetId);

#endif // JSONFUNCTIONS_H