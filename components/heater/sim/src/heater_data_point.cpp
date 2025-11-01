#include <ItcLogger/Logger.hpp>
#include <heater_data_point.hpp>
#include <cmath>

namespace Nos3
{
    extern ItcLogger::Logger *sim_logger;

    HeaterDataPoint::HeaterDataPoint(double count) : _not_parsed(false)
    {
        sim_logger->trace("HeaterDataPoint::HeaterDataPoint:  Defined Constructor executed");

        /* Provide ambient temperature with slight time variation */
        _data_is_valid = true;
        _ambient_temperature = 20.0 + 2.0 * sin(count * 0.05); // Varies 18-22°C
    }

    HeaterDataPoint::HeaterDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp) :
        _dp(*dp), _sc(spacecraft), _not_parsed(true)
    {
        sim_logger->trace("HeaterDataPoint::HeaterDataPoint:  42 Constructor executed");

        /* Initialize data */
        _data_is_valid = false;
        _ambient_temperature = 20.0;
    }

    void HeaterDataPoint::do_parsing(void) const
    {
        try {
            /*
            ** Declare 42 telemetry string prefix
            ** 42 variables defined in `42/Include/42types.h`
            ** 42 data stream defined in `42/Source/IPC/SimWriteToSocket.c`
            */
            std::string key;
            // Try to get spacecraft body temperature from 42
            // This could be extended to read actual thermal data from 42
            key.append("SC[").append(std::to_string(_sc)).append("].Temp");

            /* Parse 42 telemetry */
            std::string value = _dp.get_value_for_key(key);

            if (!value.empty()) {
                _ambient_temperature = std::stod(value);
                _data_is_valid = true;
            } else {
                // If no 42 data available, use default ambient
                _ambient_temperature = 20.0;
                _data_is_valid = true;
            }

            _not_parsed = false;

            /* Debug print */
            sim_logger->trace("HeaterDataPoint::HeaterDataPoint:  Parsed ambient temp = %.2f°C", _ambient_temperature);
        } catch (const std::exception &e) {
            sim_logger->error("HeaterDataPoint::HeaterDataPoint:  Error parsing data.  Error=%s", e.what());
            // Fall back to default
            _ambient_temperature = 20.0;
            _data_is_valid = true;
            _not_parsed = false;
        }
    }

    /* Used for printing a representation of the data point */
    std::string HeaterDataPoint::to_string(void) const
    {
        sim_logger->trace("HeaterDataPoint::to_string:  Executed");

        std::stringstream ss;

        ss << std::fixed << std::setfill(' ');
        ss << "Heater Data Point:   Valid: ";
        ss << (_data_is_valid ? "Valid" : "INVALID");
        ss << std::setprecision(2);
        ss << " Ambient Temperature: "
           << _ambient_temperature
           << "°C";

        return ss.str();
    }
} /* namespace Nos3 */
