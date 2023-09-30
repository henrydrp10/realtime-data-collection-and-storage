#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include <string>
#include <vector>

#include "dataPoint.h"

/**
 * @class DataChannel 
 * 
 * @brief DataChannel class. Belongs to a specific sensor / data source.
 * 
 * Contains information about the channel (id, name, unit) + a vector 
 * which will store the time series datapoints.
 * 
 */

class DataChannel {

    public:
        uint16_t m_id;
        std::string m_name;
        std::string m_unit;
        std::vector<DataPoint> m_data;

        DataChannel() = default;
        DataChannel(uint16_t id, const std::string& name, const std::string& unit);
        DataChannel(DataChannel&& other) noexcept;

        DataChannel& operator=(DataChannel&& other) noexcept;
};

#endif // DATACHANNEL_H