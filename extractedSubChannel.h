#ifndef EXTRACTEDSUBCHANNEL_H
#define EXTRACTEDSUBCHANNEL_H

#include <string>
#include <vector>

#include "dataChannel.h"
#include "dataPoint.h"

/**
 * @class ExtractedSubChannel
 * 
 * @brief Subset of a channel between two timestamps.
 * 
 * Class which contains information about the channel
 * (id, name, unit) and all datapoints between 2 timestamps
 * in the form of 2 different vectors (timestamps and values).
 * The reason behind this choice was to easily access a vector
 * with all the values for faster operations to the subset, and
 * leverage other functions which perform calculations on vectors
 * more efficiently. It also contains the NaN datapoints (separately
 * from the valuable data, allowing them to be analysed on their own).
 * 
 */

class ExtractedSubChannel {

    public:
        uint16_t m_id;
        std::string m_name;
        std::string m_unit;
        std::vector<double> m_timestamps;
        std::vector<double> m_values;
        std::vector<DataPoint> m_nan_dps;

        ExtractedSubChannel() = default;
        ExtractedSubChannel(const DataChannel& dataChannel, const int size);
        ExtractedSubChannel(ExtractedSubChannel&& other) noexcept;
};

#endif // EXTRACTEDSUBCHANNEL_H