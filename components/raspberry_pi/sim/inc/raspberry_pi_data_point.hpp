#ifndef NOS3_RASPBERRY_PIDATAPOINT_HPP
#define NOS3_RASPBERRY_PIDATAPOINT_HPP

#include <boost/shared_ptr.hpp>
#include <sim_42data_point.hpp>

namespace Nos3
{
    /* Standard for a data point used transfer data between a data provider and a hardware model */
    class Raspberry_piDataPoint : public Sim42DataPoint
    {
    public:
        /* Constructors */
        Raspberry_piDataPoint(double count);
        Raspberry_piDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp);

        /* Accessors */
        /* Provide the hardware model a way to get the specific data out of the data point */
        std::string to_string(void) const;
        double      get_raspberry_pi_data_x(void) const {parse_data_point(); return _raspberry_pi_data[0];}
        double      get_raspberry_pi_data_y(void) const {parse_data_point(); return _raspberry_pi_data[1];}
        double      get_raspberry_pi_data_z(void) const {parse_data_point(); return _raspberry_pi_data[2];}
        bool        is_raspberry_pi_data_valid(void) const {parse_data_point(); return _raspberry_pi_data_is_valid;}
    
    private:
        /* Disallow these */
        Raspberry_piDataPoint(void) {};
        Raspberry_piDataPoint(const Raspberry_piDataPoint& sdp) : Sim42DataPoint(sdp) {};
        ~Raspberry_piDataPoint(void) {};

        // Private mutators
        inline void parse_data_point(void) const {if (_not_parsed) do_parsing();}
        void do_parsing(void) const;

        mutable Sim42DataPoint _dp;
        int16_t _sc;
        // mutable below so parsing can be on demand:
        mutable bool _not_parsed;
        /* Specific data you need to get from the data provider to the hardware model */
        /* You only get to this data through the accessors above */
        mutable bool   _raspberry_pi_data_is_valid;
        mutable double _raspberry_pi_data[3];
    };
}

#endif
