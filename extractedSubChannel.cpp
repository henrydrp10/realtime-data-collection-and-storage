#include "dataChannel.h"
#include "extractedSubChannel.h"

ExtractedSubChannel::ExtractedSubChannel(const DataChannel& dataChannel, const int size)
    : m_id(dataChannel.m_id),
      m_name(dataChannel.m_name),
      m_unit(dataChannel.m_unit) 
    {
        m_timestamps.reserve(size);
        m_values.reserve(size);
    }

ExtractedSubChannel::ExtractedSubChannel(ExtractedSubChannel&& other) noexcept
    : m_id(other.m_id),
      m_name(std::move(other.m_name)),
      m_unit(std::move(other.m_unit)),
      m_timestamps(std::move(other.m_timestamps)),
      m_values(std::move(other.m_values)),
      m_nan_dps(std::move(other.m_nan_dps)) {}