#ifndef DATACOLLECTOR_H
#define DATACOLLECTOR_H

#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <span>
#include <thread>

#include "dataChannel.h"
#include "dataInput.h"
#include "dataPoint.h"
#include "extractedSubChannel.h"
#include "jsonFunctions.h"
#include "timer.h"

/**
 * @brief Utility functions for the main program.
 * 
 * @details Here are defined all functions that are needed for the
 * proposed solution. The main two are dataGenerator and dataCollector,
 * which do the collection and storage of the data. RetrieveChannelSubsets
 * is used for extracting the values between 2 timestamps from one or many
 * channels, and the rest are other helper / utility functions. We can also
 * find some static variables related to concurrency (as dataGenerator and
 * dataCollector can be used concurrently).
 */

static std::mutex channelsMtx;
static std::mutex queueMtx;
static std::condition_variable cv;
static bool finishedGenerating = false;

void generateDataPoint(double timestamp, std::queue<DataInput>& dataQueue, uint16_t startIndex, uint16_t endIndex);
void dataGenerator(std::queue<DataInput>& dataQueue);
void dataCollector(std::queue<DataInput>& dataQueue, std::unordered_map<uint16_t, DataChannel>& channels);
std::unordered_map<uint16_t, ExtractedSubChannel> retrieveChannelSubsets(
    std::unordered_map<uint16_t, DataChannel>& channels, const std::vector<uint16_t>& channelIds, 
    double lowerBoundTimestamp, double upperBoundTimestamp);
bool OrderedByTimestamp(const std::vector<DataPoint>& datapoints);
bool checkOrder(std::unordered_map<uint16_t, DataChannel>& channels);
bool compareByTimestamp(const DataPoint& a, const DataPoint& b);

#endif // DATACOLLECTOR_H