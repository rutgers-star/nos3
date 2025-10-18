#include <ItcLogger/Logger.hpp>
#include <raspberry_pi_data_point.hpp>

namespace Nos3
{
    extern ItcLogger::Logger *sim_logger;

    Raspberry_piDataPoint::Raspberry_piDataPoint(double count) : _not_parsed(false)
    {
        sim_logger->trace("Raspberry_piDataPoint::Raspberry_piDataPoint:  Defined Constructor executed");

        /* Do calculations based on provided data - also preparing like ADC data to checkout is obvious */
        _raspberry_pi_data_is_valid = true;
        _raspberry_pi_data[0] = (((count * 1) / 32767.0) - 32768.0);
        _raspberry_pi_data[1] = (((count * 2) / 32767.0) - 32768.0);
        _raspberry_pi_data[2] = (((count * 3) / 32767.0) - 32768.0);
    }

    Raspberry_piDataPoint::Raspberry_piDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp) : _dp(*dp), _sc(spacecraft), _not_parsed(true)
    {
        sim_logger->trace("Raspberry_piDataPoint::Raspberry_piDataPoint:  42 Constructor executed");

        /* Initialize data */
        _raspberry_pi_data_is_valid = false;
        _raspberry_pi_data[0] = _raspberry_pi_data[1] = _raspberry_pi_data[2] = 0.0;
    }
    
    void Raspberry_piDataPoint::do_parsing(void) const
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
                _raspberry_pi_data_is_valid = false;
            } else {
                _raspberry_pi_data[0] = data[0];
                _raspberry_pi_data[1] = data[1];
                _raspberry_pi_data[2] = data[2];
                /* Mark data as valid */
                _raspberry_pi_data_is_valid = true;
            }

            _not_parsed = false;

            /* Debug print */
            sim_logger->trace("Raspberry_piDataPoint::Raspberry_piDataPoint:  Parsed svb = %f %f %f", _raspberry_pi_data[0], _raspberry_pi_data[1], _raspberry_pi_data[2]);
        } catch (const std::exception &e) {
            sim_logger->error("Raspberry_piDataPoint::Raspberry_piDataPoint:  Error parsing svb.  Error=%s", e.what());
        }
    }

    /* Used for printing a representation of the data point */
    std::string Raspberry_piDataPoint::to_string(void) const
    {
        sim_logger->trace("Raspberry_piDataPoint::to_string:  Executed");
        
        std::stringstream ss;

        ss << std::fixed << std::setfill(' ');
        ss << "Raspberry_pi Data Point:   Valid: ";
        ss << (_raspberry_pi_data_is_valid ? "Valid" : "INVALID");
        ss << std::setprecision(std::numeric_limits<double>::digits10); /* Full double precision */
        ss << " Raspberry_pi Data: "
           << _raspberry_pi_data[0]
           << " "
           << _raspberry_pi_data[1]
           << " "
           << _raspberry_pi_data[2];

        return ss.str();
    }
} /* namespace Nos3 */
