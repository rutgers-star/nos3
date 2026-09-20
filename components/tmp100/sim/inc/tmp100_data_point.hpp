#ifndef NOS3_TMP100DATAPOINT_HPP
#define NOS3_TMP100DATAPOINT_HPP

#include <boost/shared_ptr.hpp>
#include <sim_42data_point.hpp>

namespace Nos3
{
    /* Standard for a data point used transfer data between a data provider and a hardware model */
    class Tmp100DataPoint : public Sim42DataPoint
    {
    public:
        /* Constructors */
        Tmp100DataPoint(double count);
        Tmp100DataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp);

        /* Accessors */
        /* Provide the hardware model a way to get the specific data out of the data point */
        std::string to_string(void) const;
        double      get_tmp100_data_x(void) const {parse_data_point(); return _tmp100_data[0];}
        double      get_tmp100_data_y(void) const {parse_data_point(); return _tmp100_data[1];}
        double      get_tmp100_data_z(void) const {parse_data_point(); return _tmp100_data[2];}
        bool        is_tmp100_data_valid(void) const {parse_data_point(); return _tmp100_data_is_valid;}
    
    private:
        /* Disallow these */
        Tmp100DataPoint(void) {};
        Tmp100DataPoint(const Tmp100DataPoint& sdp) : Sim42DataPoint(sdp) {};
        ~Tmp100DataPoint(void) {};

        // Private mutators
        inline void parse_data_point(void) const {if (_not_parsed) do_parsing();}
        void do_parsing(void) const;

        mutable Sim42DataPoint _dp;
        int16_t _sc;
        // mutable below so parsing can be on demand:
        mutable bool _not_parsed;
        /* Specific data you need to get from the data provider to the hardware model */
        /* You only get to this data through the accessors above */
        mutable bool   _tmp100_data_is_valid;
        mutable double _tmp100_data[3];
    };
}

#endif
