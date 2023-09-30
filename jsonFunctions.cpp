#include "jsonFunctions.h"

/**
 * @brief JSON - Persistence storage
 * 
 * @details Saving all data channels into a human readable
 * and easily accessible JSON file.
 */

void saveJson(const std::unordered_map<uint16_t, DataChannel>& channels) {

    Json::Value output;
    
    for (const auto& pair : channels) {

        Json::Value dataChannel;
        dataChannel["id"] = pair.second.m_id;
        dataChannel["name"] = pair.second.m_name;
        dataChannel["unit"] = pair.second.m_unit;

        Json::Value dataVector(Json::arrayValue);
        for (const auto& elem : pair.second.m_data) {
            Json::Value dataPoint;
            dataPoint["timestamp"] = elem.m_timestamp;
            dataPoint["value"] = elem.m_value;
            dataVector.append(dataPoint);
        }

        dataChannel["data"] = dataVector; 
        output[pair.first] = dataChannel;
    }

    std::ofstream outputFile("../storage/channels.json");

    Json::StreamWriterBuilder writer;
    Json::StreamWriter *jsonWriter = writer.newStreamWriter();
    jsonWriter->write(output, &outputFile);

    delete jsonWriter;
    outputFile.close();

    std::cout << "All Channels saved" << std::endl;

}


/**
 * @brief JSON - Persistence storage
 * 
 * @details Saving a specific data channel into a human readable
 * and easily accessible JSON file.
 */

void saveChannel(const DataChannel& channel) {

    Json::Value dataChannel;

    dataChannel["id"] = channel.m_id;
    dataChannel["name"] = channel.m_name;
    dataChannel["unit"] = channel.m_unit;

    Json::Value dataVector(Json::arrayValue);
    for (const auto& elem : channel.m_data) {
        Json::Value dataPoint;
        dataPoint["timestamp"] = elem.m_timestamp;
        dataPoint["value"] = elem.m_value;
        dataVector.append(dataPoint);
    }
    dataChannel["data"] = dataVector;
    
    std::ofstream outputFile("../storage/channel_" + std::to_string(channel.m_id) + ".json");
    Json::StreamWriterBuilder writer;
    Json::StreamWriter *jsonWriter = writer.newStreamWriter();
    jsonWriter->write(dataChannel, &outputFile);

    delete jsonWriter;
    outputFile.close();

    std::cout << "Channel Saved" << std::endl;

}

/**
 * @brief JSON - Persistence storage
 * 
 * @details Helper function. Loads a JSON object into a 
 * DataChannel.
 */

void loadChannel(DataChannel& channelLoaded, const Json::Value& obj) {
    DataChannel dc;
    dc.m_id = (uint16_t) obj["id"].asInt();
    dc.m_name = obj["name"].asString();
    dc.m_unit = obj["unit"].asString();

    std::vector<DataPoint> dpVec;
    const Json::Value& data = obj["data"];
    for (const Json::Value& elem : data) {
        DataPoint dp;
        dp.m_timestamp = elem["timestamp"].asDouble();
        if (elem["value"].isNull()) dp.m_value = std::numeric_limits<double>::quiet_NaN();
        else dp.m_value = elem["value"].asDouble();
        dpVec.push_back(std::move(dp));
    }

    dc.m_data = std::move(dpVec);
    channelLoaded = std::move(dc);

    return;
}

/**
 * @brief JSON - Persistence storage
 * 
 * @details Load all channels into a single JSON file.
 */

void loadJson(std::unordered_map<uint16_t, DataChannel>& channelsLoaded) {

    std::string filename = "../storage/channels.json";
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Error opening JSON file: " << filename << std::endl;
        return; 
    }

    Json::CharReaderBuilder readerBuilder;
    Json::Value root;
    std::string errs;

    if (!Json::parseFromStream(readerBuilder, inputFile, &root, &errs)) {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
        inputFile.close();
        return;
    }

    inputFile.close();

    for (const Json::Value& obj : root) {
        loadChannel(channelsLoaded[(uint16_t) obj["id"].asInt()], obj);
    }
}

/**
 * @brief JSON - Persistence storage
 * 
 * @details Loading a specific data channel from a 
 * JSON file. First checks if the channel has been saved
 * separately, in order to extract it from there. If not,
 * it searches for it in channels.json and extracts it
 * from there.
 */

void loadJsonChannel(DataChannel& channelLoaded, uint16_t targetId) {

    std::string filename = "../storage/channel_" + std::to_string(targetId) + ".json";
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {

        std::cout << "Didn't find separate JSON file" << std::endl;

        inputFile.close();
        filename = "../storage/channels.json";
        inputFile.open(filename);

        if (!inputFile.is_open()) {
            std::cerr << "Error opening JSON file: " << filename << std::endl;
            return; 
        }

        Json::CharReaderBuilder readerBuilder;
        Json::Value root;
        std::string errs;

        if (!Json::parseFromStream(readerBuilder, inputFile, &root, &errs)) {
            std::cerr << "Error parsing JSON: " << errs << std::endl;
            inputFile.close();
            return;
        }

        inputFile.close();

        for (const Json::Value& obj : root) {
            if (obj.isMember("id") && obj["id"].isInt() && obj["id"].asInt() == targetId) {
                loadChannel(channelLoaded, obj);
                return;
            }
        }

        std::cout << "No channel with that ID" << std::endl;
        return;
    }

    std::cout << "Found separate JSON file" << std::endl;

    Json::CharReaderBuilder readerBuilder;
    Json::Value root;
    std::string errs;

    if (!Json::parseFromStream(readerBuilder, inputFile, &root, &errs)) {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
        inputFile.close();
        return;
    }

    inputFile.close();
    loadChannel(channelLoaded, root);
}
