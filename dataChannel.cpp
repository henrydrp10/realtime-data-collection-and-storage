#include "dataChannel.h"

DataChannel::DataChannel(uint16_t id, const std::string& name, const std::string& unit)
    : m_id(id), m_name(name), m_unit(unit) {
        m_data.reserve(20000);
    }

DataChannel::DataChannel(DataChannel&& other) noexcept 
    : m_id(other.m_id),
      m_name(std::move(other.m_name)),
      m_unit(std::move(other.m_unit)),
      m_data(std::move(other.m_data)) {}

DataChannel& DataChannel::operator=(DataChannel&& other) noexcept {
    if (this != &other) {
        m_id = other.m_id;
        m_name = std::move(other.m_name),
        m_unit = std::move(other.m_unit),
        m_data = std::move(other.m_data),
        other.m_id = 0;
        other.m_name.clear();
        other.m_unit.clear();
        other.m_data.clear(); {}
    }
    return *this;
}