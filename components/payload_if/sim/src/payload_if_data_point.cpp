#include <ItcLogger/Logger.hpp>
#include <payload_if_data_point.hpp>

namespace Nos3
{
    extern ItcLogger::Logger *sim_logger;

    Payload_ifDataPoint::Payload_ifDataPoint(double count) : _not_parsed(false)
    {
        sim_logger->trace("Payload_ifDataPoint::Payload_ifDataPoint:  Defined Constructor executed");

        /* Do calculations based on provided data - also preparing like ADC data to checkout is obvious */
        _payload_if_data_is_valid = true;
        _payload_if_data[0] = (((count * 1) / 32767.0) - 32768.0);
        _payload_if_data[1] = (((count * 2) / 32767.0) - 32768.0);
        _payload_if_data[2] = (((count * 3) / 32767.0) - 32768.0);
    }

    Payload_ifDataPoint::Payload_ifDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp) : _dp(*dp), _sc(spacecraft), _not_parsed(true)
    {
        sim_logger->trace("Payload_ifDataPoint::Payload_ifDataPoint:  42 Constructor executed");

        /* Initialize data */
        _payload_if_data_is_valid = false;
        _payload_if_data[0] = _payload_if_data[1] = _payload_if_data[2] = 0.0;
    }
    
    void Payload_ifDataPoint::do_parsing(void) const
    {
        try {
            /*
            ** Declare 42 telemetry string prefix
            ** 42 variables defined in `42/Include/42types.h`
            ** 42 data stream defined in `42/Source/IPC/SimWriteToSocket.c`
            */
            std::string key;
            key.append("SC[").append(std::to_string(_sc)).append("].svb"); // SC[N].svb

            /* Parse 42 telemetry */
            std::string values = _dp.get_value_for_key(key);

            std::vector<double> data;
            data.reserve(3);
            parse_double_vector(values, data);

            if (data.size() < 3) {
                _payload_if_data_is_valid = false;
            } else {
                _payload_if_data[0] = data[0];
                _payload_if_data[1] = data[1];
                _payload_if_data[2] = data[2];
                /* Mark data as valid */
                _payload_if_data_is_valid = true;
            }

            _not_parsed = false;

            /* Debug print */
            sim_logger->trace("Payload_ifDataPoint::Payload_ifDataPoint:  Parsed svb = %f %f %f", _payload_if_data[0], _payload_if_data[1], _payload_if_data[2]);
        } catch (const std::exception &e) {
            sim_logger->error("Payload_ifDataPoint::Payload_ifDataPoint:  Error parsing svb.  Error=%s", e.what());
        }
    }

    /* Used for printing a representation of the data point */
    std::string Payload_ifDataPoint::to_string(void) const
    {
        sim_logger->trace("Payload_ifDataPoint::to_string:  Executed");
        
        std::stringstream ss;

        ss << std::fixed << std::setfill(' ');
        ss << "Payload_if Data Point:   Valid: ";
        ss << (_payload_if_data_is_valid ? "Valid" : "INVALID");
        ss << std::setprecision(std::numeric_limits<double>::digits10); /* Full double precision */
        ss << " Payload_if Data: "
           << _payload_if_data[0]
           << " "
           << _payload_if_data[1]
           << " "
           << _payload_if_data[2];

        return ss.str();
    }
} /* namespace Nos3 */
