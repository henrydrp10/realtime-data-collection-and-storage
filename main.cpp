#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <queue>
#include <random>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include "dataChannel.h"
#include "dataCollector.h"
#include "dataInput.h"
#include "dataPoint.h"
#include "extractedSubChannel.h"
#include "jsonFunctions.h"
#include "timer.h"

/**
 * @brief Macro definition. Use of a 
 * thread pool for data collection.
 */
// #define THREAD_POOL false

// extern std::mutex printMtx;
// extern std::mutex queueMtx;
// extern std::condition_variable cv;
// extern bool finishedGenerating;

// #if THREAD_POOL
// extern std::mutex channelsMtx;
// #endif

/**
 * @brief Main function.
 * 
 * @return int 
 * 
 * @details Showcase the different methods implemented.
 */

int main() {

    std::cout << std::endl;
    std::cout << "----------------------- STARTING PROGRAM -------------------------" << std::endl;
    std::cout << std::endl;

    std::unordered_map<uint16_t, DataChannel> channels;
    std::queue<DataInput> dataQueue;

    std::thread genThread(dataGenerator, std::ref(dataQueue));

    bool threadPool = false;

    if (threadPool) {

        std::this_thread::sleep_for(std::chrono::seconds(1));
        const int numColThreads = std::thread::hardware_concurrency() - 2;
        std::cout << std::endl;
        std::cout << "Number of threads to participate in the thread pool: " << numColThreads << std::endl;
        std::cout << std::endl;
        std::vector<std::thread> colThreads;
        for (int i = 0; i < numColThreads; i++) {
            colThreads.emplace_back(dataCollector, std::ref(dataQueue), std::ref(channels));
        }
        genThread.join();
        for (auto& thread : colThreads) thread.join();

        bool ordered = checkOrder(channels);
        std::cout << std::endl;
        if (ordered) std::cout << "The datapoints from the channels are ordered!" << std::endl;
        else std::cout << "The datapoints from the channels are not ordered! (Due to thread pool race conditions)" << std::endl;

        {
            Timer timer("sorting channels");
            for (auto& channel : channels) {
                std::sort(
                    channel.second.m_data.begin(),
                    channel.second.m_data.end(),
                    compareByTimestamp
                );
            }
        }

        ordered = checkOrder(channels);
        std::cout << std::endl;
        if (ordered) std::cout << "The datapoints from the channels are ordered!" << std::endl;
        else std::cout << "The datapoints from the channels are not ordered! (Due to thread pool race conditions)" << std::endl;

    } else {

        std::cout << std::endl;
        std::cout << "No thread pool, just 1 thread for data collection." << std::endl;
        std::cout << std::endl;
        std::thread colThread(dataCollector, std::ref(dataQueue), std::ref(channels));
        genThread.join();
        colThread.join();

        bool ordered = checkOrder(channels);
        std::cout << std::endl;
        if (ordered) std::cout << "The datapoints from the channels are ordered!" << std::endl;
        else std::cout << "The datapoints from the channels are not ordered! (Due to thread pool race conditions)" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "The data queue has " << dataQueue.size() << " elements after both functions are done!" << std::endl;
    std::cout << std::endl; 

    // Print length of some channels
    std::cout << "----------------------- EXAMPLE DATA CHANNELS -------------------------" << std::endl;
    std::cout << std::endl;
    std::vector<uint16_t> channelIdsToShow = {15, 40, 65, 85};
    for (uint16_t id : channelIdsToShow) {
        std::cout << "Channel " << id << ": " << std::endl;
        std::cout << "Name: " << channels[id].m_name << std::endl;
        std::cout << "Unit: " << channels[id].m_unit << std::endl;
        std::cout << "Datapoint vector size: " << channels[id].m_data.size() << std::endl;
        std::cout << std::endl;
    }

    std::cout << "----------------------- RETRIEVING CHANNEL SUBSETS -------------------------" << std::endl;
    std::cout << std::endl;

    // List of ids to extract subsets from
    std::vector<uint16_t> channelIds = {3, 16, 27, 45, 60, 68, 79, 91};
    std::cout << "Channels to extract subsets from: " << std::endl << "{";
    for (auto it = channelIds.begin(); it != std::prev(channelIds.end()); it++) {
        std::cout << *it << ", ";
    }
    std::cout << *(std::prev(channelIds.end())) << "}" << std::endl << std::endl;

    double lowerTs = 10000;
    double upperTs = 20000;
    std::cout << "Extract datapoints between " << lowerTs << " ms and " << upperTs << " ms." << std::endl;
    std::cout << std::endl;

    std::unordered_map<uint16_t, ExtractedSubChannel> subsetChannels = retrieveChannelSubsets(channels, channelIds, lowerTs, upperTs);

    std::cout << std::endl;
    for (uint16_t id : channelIds) {
        std::cout << "Extracted Channel " << id << ": " << std::endl;
        std::cout << "Name: " << subsetChannels[id].m_name << std::endl;
        std::cout << "Unit: " << subsetChannels[id].m_unit << std::endl;
        std::cout << "Timestamps vector size: " << subsetChannels[id].m_timestamps.size() << std::endl;
        std::cout << "Values vector size: " << subsetChannels[id].m_timestamps.size() << std::endl;
        std::cout << "Average value: " << std::accumulate(
            subsetChannels[id].m_values.begin(),
            subsetChannels[id].m_values.end(),
            0.0
        ) / subsetChannels[id].m_values.size() << std::endl;
        std::cout << "Number of omitted datapoints (NaN values): " << subsetChannels[id].m_nan_dps.size() << "  ";
        for (auto& dp : subsetChannels[id].m_nan_dps) std::cout << "(" << dp.m_timestamp << ", " << dp.m_value << "), ";
        std::cout << std::endl << std::endl;
    }

    std::cout << "----------------------- SAVING TO PERSISTENT STORAGE -------------------------" << std::endl;
    std::cout << std::endl;

    std::cout << "Saving all channels..." << std::endl;
    saveJson(channels);
    std::cout << std::endl;

    uint16_t channelToSave = 32;
    std::cout << "Saving channel " << channelToSave << std::endl;
    saveChannel(channels[channelToSave]);
    std::cout << std::endl;

    std::unordered_map<uint16_t, DataChannel> channelsLoaded;

    DataChannel channelLoaded;
    std::cout << "Loading all channels..." << std::endl;
    loadJson(channelsLoaded);
    std::cout << std::endl;

    std::cout << "Loading channel " << channelToSave << " from the file..." << std::endl;
    loadJsonChannel(channelLoaded, 32);
    std::cout << std::endl;

    DataChannel channelLoaded2;
    std::cout << "Loading channel " << channelToSave << " (not in a specific file)" << std::endl;
    loadJsonChannel(channelLoaded2, 65);
    std::cout << std::endl;

    std::cout << "Printing channel 32 (loaded with all channels)" << std::endl;
    std::cout << "ID: " << channelsLoaded[32].m_id << ", ";
    std::cout << "Name: " << channelsLoaded[32].m_name << ", ";
    std::cout << "Unit: " << channelsLoaded[32].m_unit << ", ";
    std::cout << "Datapoints vector size: " << channelsLoaded[32].m_data.size() << ", ";
    std::cout << "First datapoint: (" << channelsLoaded[32].m_data[0].m_timestamp << ", ";
    std::cout << channelsLoaded[32].m_data[0].m_value << ")" << std::endl;
    std::cout << std::endl;

    std::cout << "Printing channel 32 (loaded from file)" << std::endl;
    std::cout << "ID: " << channelLoaded.m_id << ", ";
    std::cout << "Name: " << channelLoaded.m_name << ", ";
    std::cout << "Unit: " << channelLoaded.m_unit << ", ";
    std::cout << "Datapoints vector size: " << channelLoaded.m_data.size() << ", ";
    std::cout << "First datapoint: (" << channelLoaded.m_data[0].m_timestamp << ", ";
    std::cout << channelLoaded.m_data[0].m_value << ")" << std::endl;
    std::cout << std::endl;

    std::cout << "Printing channel 65 (loaded with all channels)" << std::endl;
    std::cout << "ID: " << channelsLoaded[65].m_id << ", ";
    std::cout << "Name: " << channelsLoaded[65].m_name << ", ";
    std::cout << "Unit: " << channelsLoaded[65].m_unit << ", ";
    std::cout << "Datapoints vector size: " << channelsLoaded[65].m_data.size() << ", ";
    std::cout << "First datapoint: (" << channelsLoaded[65].m_data[0].m_timestamp << ", ";
    std::cout << channelsLoaded[65].m_data[0].m_value << ")" << std::endl;
    std::cout << std::endl;

    std::cout << "Printing channel 65 (not in a specific file, hence extracted from all channels)" << std::endl;
    std::cout << "ID: " << channelLoaded2.m_id << ", ";
    std::cout << "Name: " << channelLoaded2.m_name << ", ";
    std::cout << "Unit: " << channelLoaded2.m_unit << ", ";
    std::cout << "Datapoints vector size: " << channelLoaded2.m_data.size() << ", ";
    std::cout << "First datapoint: (" << channelLoaded2.m_data[0].m_timestamp << ", ";
    std::cout << channelLoaded2.m_data[0].m_value << ")" << std::endl;
    std::cout << std::endl;

    return 0;
}