#include <ItcLogger/Logger.hpp>
#include <tmp100_data_point.hpp>
#include <cmath>

namespace Nos3
{
    extern ItcLogger::Logger *sim_logger;

    Tmp100DataPoint::Tmp100DataPoint(double count) : _not_parsed(false)
    {
        sim_logger->trace("Tmp100DataPoint::Tmp100DataPoint:  Defined Constructor executed");

        /* Provide a simple time-varying temperature between 20-30°C */
        _tmp100_data_is_valid = true;
        _tmp100_data[0] = 25.0 + 5.0 * sin(count * 0.1);  // Temperature varies between 20-30°C
        _tmp100_data[1] = 0.0;  // Unused
        _tmp100_data[2] = 0.0;  // Unused
    }

    Tmp100DataPoint::Tmp100DataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp) : _dp(*dp), _sc(spacecraft), _not_parsed(true)
    {
        sim_logger->trace("Tmp100DataPoint::Tmp100DataPoint:  42 Constructor executed");

        /* Initialize data */
        _tmp100_data_is_valid = false;
        _tmp100_data[0] = _tmp100_data[1] = _tmp100_data[2] = 0.0;
    }
    
    void Tmp100DataPoint::do_parsing(void) const
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
                _tmp100_data_is_valid = false;
            } else {
                _tmp100_data[0] = data[0];
                _tmp100_data[1] = data[1];
                _tmp100_data[2] = data[2];
                /* Mark data as valid */
                _tmp100_data_is_valid = true;
            }

            _not_parsed = false;

            /* Debug print */
            sim_logger->trace("Tmp100DataPoint::Tmp100DataPoint:  Parsed svb = %f %f %f", _tmp100_data[0], _tmp100_data[1], _tmp100_data[2]);
        } catch (const std::exception &e) {
            sim_logger->error("Tmp100DataPoint::Tmp100DataPoint:  Error parsing svb.  Error=%s", e.what());
        }
    }

    /* Used for printing a representation of the data point */
    std::string Tmp100DataPoint::to_string(void) const
    {
        sim_logger->trace("Tmp100DataPoint::to_string:  Executed");
        
        std::stringstream ss;

        ss << std::fixed << std::setfill(' ');
        ss << "Tmp100 Data Point:   Valid: ";
        ss << (_tmp100_data_is_valid ? "Valid" : "INVALID");
        ss << std::setprecision(std::numeric_limits<double>::digits10); /* Full double precision */
        ss << " Tmp100 Data: "
           << _tmp100_data[0]
           << " "
           << _tmp100_data[1]
           << " "
           << _tmp100_data[2];

        return ss.str();
    }
} /* namespace Nos3 */
