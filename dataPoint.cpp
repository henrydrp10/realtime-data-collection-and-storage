#include "dataPoint.h"

DataPoint::DataPoint(double timestamp, double val)
    : m_timestamp(timestamp), m_value(val) {}

DataPoint::DataPoint(const DataPoint& other)
    : m_timestamp(other.m_timestamp), m_value(other.m_value) {}

DataPoint::DataPoint(DataPoint&& other) noexcept
    : m_timestamp(other.m_timestamp), m_value(other.m_value) {}

DataPoint& DataPoint::operator=(DataPoint&& other) {
    if (this != &other) {
        m_timestamp = other.m_timestamp;
        m_value = other.m_value;
        other.m_timestamp = 0;
        other.m_value = 0;
    }
    return *this;
}

bool DataPoint::operator<(const DataPoint& other) const {
        return m_timestamp < other.m_timestamp;
    }