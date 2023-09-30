#ifndef DATAINPUT_H
#define DATAINPUT_H

#include <string>

#include "dataPoint.h"

/**
 * @class DataInput
 * 
 * @brief DataInput. Data point coming from the data source.
 * 
 * Class created to simulate a format in which a data point
 * might arrive to the collection program (in order to be stored).
 * 
 */

class DataInput {

    public:
        uint16_t m_id;
        std::string m_name;
        std::string m_unit;
        DataPoint m_dp;
        
        DataInput() = default;
        DataInput(uint16_t id, const std::string& name, const std::string& unit, DataPoint&& other) noexcept;
        DataInput(DataInput&& other) noexcept;
        DataInput& operator=(DataInput&& other);
};

#endif // DATAINPUT_H