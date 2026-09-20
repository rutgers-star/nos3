#ifndef NOS3_HEATERDATAPOINT_HPP
#define NOS3_HEATERDATAPOINT_HPP

#include <boost/shared_ptr.hpp>
#include <sim_42data_point.hpp>

namespace Nos3
{
    /* Standard for a data point used transfer data between a data provider and a hardware model */
    class HeaterDataPoint : public Sim42DataPoint
    {
    public:
        /* Constructors */
        HeaterDataPoint(double count);
        HeaterDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp);

        /* Accessors */
        /* Provide the hardware model a way to get the specific data out of the data point */
        std::string to_string(void) const;
        double      get_ambient_temperature(void) const {parse_data_point(); return _ambient_temperature;}
        bool        is_data_valid(void) const {parse_data_point(); return _data_is_valid;}

    private:
        /* Disallow these */
        HeaterDataPoint(void) {};
        HeaterDataPoint(const HeaterDataPoint& sdp) : Sim42DataPoint(sdp) {};
        ~HeaterDataPoint(void) {};

        // Private mutators
        inline void parse_data_point(void) const {if (_not_parsed) do_parsing();}
        void do_parsing(void) const;

        mutable Sim42DataPoint _dp;
        int16_t _sc;
        // mutable below so parsing can be on demand:
        mutable bool _not_parsed;
        /* Specific data you need to get from the data provider to the hardware model */
        /* You only get to this data through the accessors above */
        mutable bool   _data_is_valid;
        mutable double _ambient_temperature;
    };
}

#endif
