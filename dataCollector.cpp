#include "dataCollector.h"

/**
 * @brief DataPoint creation and insertion into the data queue.
 * 
 * @param timestamp 
 * @param dataQueue 
 * @param startIndex 
 * @param endIndex
 * 
 * @details Generates a random value and receives a timestamp in
 * order to create a DataPoint and insert it into the appropriate
 * channels (from channel startIndex to channel endIndex). 
 */

void generateDataPoint(
    double timestamp,
    std::queue<DataInput>& dataQueue, 
    uint16_t startIndex, 
    uint16_t endIndex) 
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> valueDist(0.0, 1.0);
    static double probability = 0.005;

    for (uint16_t index = startIndex; index < endIndex; index++) {
        double randomValue = valueDist(gen);
        if (std::bernoulli_distribution(probability)(gen))
            randomValue = std::numeric_limits<double>::quiet_NaN();
        {
            std::unique_lock<std::mutex> lock(queueMtx);
            DataPoint dp(timestamp, randomValue);
            dataQueue.emplace(
                index, 
                "Sensor_" + std::to_string(index), 
                "Unit_" + std::to_string(index),
                std::move(dp)
            );
        }
    }
}

/**
 * @brief Data Generator solution.
 * 
 * @param dataQueue 
 * 
 * @details Design solution: Implement a data queue, in order
 * to use it as the input data source pipe, where the program will
 * collect each data point and store it appropriately. Simulate
 * sample rates by using sleep and modulo functions. Generate
 * input data for a defined amount of time (desiredDuration).
 */

void dataGenerator(std::queue<DataInput>& dataQueue) {

    Timer timer("data generator");

    // Start time & amount of time for data generation
    auto startTime = std::chrono::high_resolution_clock::now();
    double desiredDuration = 30000.0; // in milliseconds

    int count = 1;
    while (true) {

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime);
        double timestamp = static_cast<double>(elapsed.count()) / 1e3;
        if (timestamp >= desiredDuration) break;

        // Sensor sample rates
        if (count % 1 == 0) generateDataPoint(timestamp, dataQueue, 0, 25); // 100hz
        if (count % 2 == 0) generateDataPoint(timestamp, dataQueue, 25, 50); // 50hz
        if (count % 5 == 0) generateDataPoint(timestamp, dataQueue, 50, 75); // 25hz
        if (count % 10 == 0) generateDataPoint(timestamp, dataQueue, 75, 101); // 10hz

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        count += 1;
    }
    finishedGenerating = true;
    cv.notify_all();

}

/**
 * @brief Data Collector solution.
 * 
 * @param dataQueue 
 * @param channels 
 * 
 * @details Function in charge of retrieving the first element
 * from the data queue and store it in its respective
 * channel. With the macro thread_pool we can specify if
 * we want 1 thread performing the collection, or many
 * concurrently.
 */

void dataCollector(
    std::queue<DataInput>& dataQueue,
    std::unordered_map<uint16_t, DataChannel>& channels) 
{
    Timer timer("data collector");

    while (true) {

        DataInput toMove;
        {
            std::unique_lock<std::mutex> lock(queueMtx);
            cv.wait(lock, [&] { return finishedGenerating || !dataQueue.empty(); });
            if (finishedGenerating && dataQueue.empty()) break; 
            toMove = std::move(dataQueue.front());
            dataQueue.pop();
        }
        {
            std::unique_lock<std::mutex> lock(channelsMtx);
            if (channels.find(toMove.m_id) == channels.end()) {
                DataChannel dc(toMove.m_id, toMove.m_name, toMove.m_unit);
                channels.emplace(toMove.m_id, std::move(dc));
            }
            channels[toMove.m_id].m_data.push_back(std::move(toMove.m_dp));
        }
    }
}

/**
 * @brief Retrieves subsets of channels (between 2 timestamps).
 * 
 * @param channels 
 * @param channelIds 
 * @param lowerBoundTimestamp 
 * @param upperBoundTimestamp 
 * @return std::unordered_map<uint16_t, ExtractedSubChannel>
 * 
 * @details Recieves a list of channel Ids and two timestamps.
 * Extracts the datapoints between the two timestamps and returns
 * it in a structured way more suitable for calculations on the 
 * values. Also handles NaN values by omitting them from the
 * output. Returns a map of the extracted subsets from each channel.
 */

std::unordered_map<uint16_t, ExtractedSubChannel> retrieveChannelSubsets(
    std::unordered_map<uint16_t, DataChannel>& channels, 
    const std::vector<uint16_t>& channelIds, 
    double lowerBoundTimestamp, 
    double upperBoundTimestamp) 
{
    Timer timer("subset retrieval");
    std::unordered_map<uint16_t, ExtractedSubChannel> subsetChannels;

    for (uint16_t channelId : channelIds) {

        auto startIt = std::lower_bound(
            channels[channelId].m_data.begin(), 
            channels[channelId].m_data.end(), 
            lowerBoundTimestamp,
            [](const DataPoint& dp, double t) { 
                return dp.m_timestamp < t; 
            }
        );

        auto endIt = std::upper_bound(
            channels[channelId].m_data.begin(), 
            channels[channelId].m_data.end(), 
            upperBoundTimestamp,
            [](double t, const DataPoint& dp) { 
                return t < dp.m_timestamp; 
            }
        );

        std::span<DataPoint> subsequence_span(startIt, endIt);
        ExtractedSubChannel subChannel(channels[channelId], subsequence_span.size());

        for (const DataPoint& dp : subsequence_span) {
            if (!std::isnan(dp.m_value)) {
                subChannel.m_timestamps.push_back(dp.m_timestamp);
                subChannel.m_values.push_back(dp.m_value);
            } else {
                subChannel.m_nan_dps.push_back(dp);
            }
        }

        subsetChannels.emplace(channelId, std::move(subChannel));
    }
    return subsetChannels;
}

/**
 * @brief Check that the vector of datapoints is sorted.
 * 
 * @param datapoints 
 * @return true 
 * @return false 
 * 
 * @details Loops through the datapoint vector and compares
 * timestamps.
 */

bool OrderedByTimestamp(const std::vector<DataPoint>& datapoints) {
    if (datapoints.empty()) return true;
    for (size_t i = 1; i < datapoints.size(); i++) {
        if (datapoints[i - 1].m_timestamp >= datapoints[i].m_timestamp) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Checks that all data channels have their DataPoints ordered.
 * 
 * @param channels 
 * @return true 
 * @return false
 * 
 * @details Checks that the vector of datapoints is sorted.
 * In the thread pool case, due to race conditions, there is
 * no certainty that the datapoints will be inserted in order,
 * hence using a thread pool for data collection does not
 * ensure order.
 */

bool checkOrder(std::unordered_map<uint16_t, DataChannel>& channels) {
    for (auto& channel : channels) {
        if (!OrderedByTimestamp(channel.second.m_data)) return false;
    }
    return true;
}

/**
 * @brief Compare 2 datapoints using their timestamps.
 * 
 * @param a 
 * @param b 
 * @return true 
 * @return false
 * 
 * @details Helper function for std::sort with DataPoint 
 * structs.
 */

bool compareByTimestamp(const DataPoint& a, const DataPoint& b) {
    return a.m_timestamp < b.m_timestamp;
}