#ifndef DATAPOINT_H
#define DATAPOINT_H

/**
 * @class DataPoint
 * 
 * @brief DataPoint class.
 * 
 * Contains a timestamp and a value. Public member
 * variables mainly for easier access from the main
 * function.
 * 
 */

class DataPoint {

    public:
        double m_timestamp;
        double m_value;
    
        DataPoint() = default;
        DataPoint(double timestamp, double val);
        DataPoint(const DataPoint& other);
        DataPoint(DataPoint&& other) noexcept;

        DataPoint& operator=(DataPoint&& other);
        bool operator<(const DataPoint& other) const;
};

#endif // DATAPOINT_H