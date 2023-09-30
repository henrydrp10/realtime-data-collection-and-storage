#include "dataInput.h"

DataInput::DataInput(uint16_t id, const std::string& name, const std::string& unit, DataPoint&& other) noexcept
    : m_id(id), m_name(name), m_unit(unit) {
        m_dp.m_timestamp = other.m_timestamp;
        m_dp.m_value = other.m_value;
    }

DataInput::DataInput(DataInput&& other) noexcept
    : m_id(other.m_id),
        m_name(std::move(other.m_name)),
        m_unit(std::move(other.m_unit)),
        m_dp(std::move(other.m_dp)) {}

DataInput& DataInput::operator=(DataInput&& other) {
    if (this != &other) {
        m_id = other.m_id;
        m_name = std::move(other.m_name);
        m_unit = std::move(other.m_unit);
        m_dp = std::move(other.m_dp);
    
        other.m_id = 0;
        other.m_name.clear();
        other.m_unit.clear();
        other.m_dp = DataPoint(0, 0);
    }
    return *this;
}