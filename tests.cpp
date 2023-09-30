#include <queue>
#include <thread>

#include <gtest/gtest.h>

#include "dataChannel.h"
#include "dataCollector.h"
#include "dataInput.h"
#include "dataPoint.h"
#include "extractedSubChannel.h"
#include "jsonFunctions.h"

// Create a test suite for the DataPoint class
TEST(DataPointTest, Constructors) {

    DataPoint dp1(1.0, 42.0);
    ASSERT_EQ(dp1.m_timestamp, 1.0);
    ASSERT_EQ(dp1.m_value, 42.0);

    DataPoint dp2(dp1);
    ASSERT_EQ(dp2.m_timestamp, dp1.m_timestamp);
    ASSERT_EQ(dp2.m_value, dp1.m_value);
}

// Create a test suite for the DataChannel class
TEST(DataChannelTest, Constructors) {

    DataChannel channel1(1, "Sensor_1", "Unit_1");
    ASSERT_EQ(channel1.m_id, 1);
    ASSERT_EQ(channel1.m_name, "Sensor_1");
    ASSERT_EQ(channel1.m_unit, "Unit_1");
    ASSERT_TRUE(channel1.m_data.empty());

    DataPoint dp(1.0, 42.0);
    DataChannel channel2(2, "Sensor_2", "Unit_2");
    channel2.m_data.push_back(dp);
    ASSERT_EQ(channel2.m_id, 2);
    ASSERT_EQ(channel2.m_name, "Sensor_2");
    ASSERT_EQ(channel2.m_unit, "Unit_2");
    ASSERT_EQ(channel2.m_data.size(), 1);
    ASSERT_EQ(channel2.m_data[0].m_timestamp, 1.0);
    ASSERT_EQ(channel2.m_data[0].m_value, 42.0);
}

// Test suite for the DataInput class
TEST(DataInputTest, Constructors) {

    DataPoint dp(1.0, 10.0);
    DataInput di1(1, "Sensor_1", "Unit_1", std::move(dp));
    ASSERT_EQ(di1.m_id, 1);
    ASSERT_EQ(di1.m_name, "Sensor_1");
    ASSERT_EQ(di1.m_unit, "Unit_1");
    ASSERT_EQ(di1.m_dp.m_timestamp, 1.0);
    ASSERT_EQ(di1.m_dp.m_value, 10.0);
}

// Test suite for the ExtractedSubChannel class
TEST(ExtractedSubChannelTest, Constructors) {
    DataChannel channel1(1, "Sensor_1", "Unit_1");
    DataPoint dp1(1.0, 42.0);
    DataPoint dp2(2.0, 43.0);
    channel1.m_data.push_back(dp1);
    channel1.m_data.push_back(dp2);

    ExtractedSubChannel subChannel1(channel1, 2);
    ASSERT_EQ(subChannel1.m_id, 1);
    ASSERT_EQ(subChannel1.m_name, "Sensor_1");
    ASSERT_EQ(subChannel1.m_unit, "Unit_1");
    ASSERT_TRUE(subChannel1.m_timestamps.empty());
    ASSERT_TRUE(subChannel1.m_values.empty());
    ASSERT_TRUE(subChannel1.m_nan_dps.empty());
}

// Test suite for the generateDataPoint function
TEST(GenerateDataPointTest, Basic) {
    std::queue<DataInput> dataQueue;
    generateDataPoint(1.0, dataQueue, 0, 1); // Generate a data point for one channel
    ASSERT_EQ(dataQueue.size(), 1);
    ASSERT_EQ(dataQueue.front().m_dp.m_timestamp, 1.0);

    generateDataPoint(2.0, dataQueue, 0, 2); // Generate a data point for two channels
    ASSERT_EQ(dataQueue.size(), 3);
    ASSERT_EQ(dataQueue.back().m_dp.m_timestamp, 2.0);
}

// Test suite for the main functionality (generation + collection) - sequential
TEST(GenerateAndCollectTest, Basic) {

    std::unordered_map<uint16_t, DataChannel> channels;
    std::queue<DataInput> dataQueue;

    dataGenerator(dataQueue);
    ASSERT_GT(dataQueue.size(), 0);
    dataCollector(dataQueue, channels);
    ASSERT_EQ(dataQueue.size(), 0);
    ASSERT_TRUE(checkOrder(channels));

}

// Test suite for the method retrieveChannelSubsets 
TEST(RetrieveChannelSubsetsTest, Basic) {
    std::unordered_map<uint16_t, DataChannel> channels;
    DataChannel channel1(1, "Sensor_1", "Unit_1");
    DataChannel channel2(2, "Sensor_2", "Unit_2");
    DataPoint dp1(1.0, 42.0);
    DataPoint dp2(1.5, 43.0);
    DataPoint dp3(2.0, std::numeric_limits<double>::quiet_NaN());
    channel1.m_data.push_back(dp1);
    channel2.m_data.push_back(dp2);
    channel2.m_data.push_back(dp3);
    channels[1] = std::move(channel1);
    channels[2] = std::move(channel2);
    std::vector<uint16_t> channelIds = {1, 2};
    auto subsetChannels = retrieveChannelSubsets(channels, channelIds, 0.0, 3.0);
    ASSERT_TRUE(checkOrder(channels));
    ASSERT_EQ(subsetChannels.size(), channelIds.size());
    ASSERT_EQ(subsetChannels[1].m_timestamps.size(), 1);
    ASSERT_EQ(subsetChannels[2].m_timestamps.size(), 1);
    ASSERT_EQ(subsetChannels[1].m_values.size(), 1);
    ASSERT_EQ(subsetChannels[2].m_values.size(), 1);
    ASSERT_TRUE(subsetChannels[1].m_nan_dps.empty());
    ASSERT_EQ(subsetChannels[2].m_nan_dps.size(), 1);
}

// Test suite for saving and loading all channels
TEST(JsonTests, SaveAndLoadJson) {

    std::unordered_map<uint16_t, DataChannel> testChannels;

    DataChannel dc1(1, "Sensor_1", "Unit_1");
    DataPoint dp1(1.0, 10.0);
    DataPoint dp2(2.0, 11.0);
    dc1.m_data.push_back(dp1);
    dc1.m_data.push_back(dp2);

    DataChannel dc2(2, "Sensor_2", "Unit_2");
    DataPoint dp3(2.5, std::numeric_limits<double>::quiet_NaN());
    dc2.m_data.push_back(dp3);

    testChannels[1] = std::move(dc1);
    testChannels[2] = std::move(dc2);

    saveJson(testChannels);

    std::unordered_map<uint16_t, DataChannel> loadedChannels;
    loadJson(loadedChannels);

    for (auto& pair : testChannels) {
        ASSERT_NE(loadedChannels.find(pair.first), loadedChannels.end());
        ASSERT_EQ(pair.second.m_name, loadedChannels[pair.first].m_name);
        ASSERT_EQ(pair.second.m_unit, loadedChannels[pair.first].m_unit);
        ASSERT_EQ(pair.second.m_data.size(), loadedChannels[pair.first].m_data.size());
    }
}

bool customEquality(double a, double b) {
    if (std::isnan(a) && std::isnan(b)) {
        return true;
    }
    return a == b;
}

// Test suite for saving and loading a specific channel from/to JSON
TEST(JsonTests, SaveAndLoadChannel) {

    DataChannel testChannel(1, "Sensor_1", "Unit_1");
    DataPoint dp1(1.0, 42.0);
    DataPoint dp2(1.5, 43.0);
    DataPoint dp3(2.0, std::numeric_limits<double>::quiet_NaN());
    DataPoint dp4(3.0, 39.0);
    DataPoint dp5(5.0, 40.0);
    testChannel.m_data.push_back(dp1);
    testChannel.m_data.push_back(dp2);
    testChannel.m_data.push_back(dp3);
    testChannel.m_data.push_back(dp4);
    testChannel.m_data.push_back(dp5);
    saveChannel(testChannel);

    DataChannel loadedChannel;
    loadJsonChannel(loadedChannel, testChannel.m_id);

    ASSERT_EQ(testChannel.m_id, loadedChannel.m_id);
    ASSERT_EQ(testChannel.m_name, loadedChannel.m_name);
    ASSERT_EQ(testChannel.m_unit, loadedChannel.m_unit);
    ASSERT_EQ(testChannel.m_data.size(), loadedChannel.m_data.size());
    for (auto i = 0; i < testChannel.m_data.size(); i++) {
        ASSERT_EQ(testChannel.m_data[i].m_timestamp, loadedChannel.m_data[i].m_timestamp);
        ASSERT_TRUE(customEquality(testChannel.m_data[i].m_value, loadedChannel.m_data[i].m_value));
    } 
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}