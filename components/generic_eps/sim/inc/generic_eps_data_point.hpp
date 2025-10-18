#ifndef NOS3_GENERIC_EPSDATAPOINT_HPP
#define NOS3_GENERIC_EPSDATAPOINT_HPP

#include <boost/shared_ptr.hpp>
#include <sim_42data_point.hpp>

namespace Nos3
{
    /* Standard for a data point used transfer data between a data provider and a hardware model */
    class Generic_epsDataPoint : public Sim42DataPoint
    {
    public:
        /* Constructors */
        Generic_epsDataPoint(double count);
        Generic_epsDataPoint(int16_t orbit, int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp);

        /* Accessors */
        /* Provide the hardware model a way to get the specific data out of the data point */
        std::string to_string(void) const;
        double      get_sun_vector_x(void) const {parse_data_point(); return _sun_vector[0];}
        double      get_sun_vector_y(void) const {parse_data_point(); return _sun_vector[1];}
        double      get_sun_vector_z(void) const {parse_data_point(); return _sun_vector[2];}
        bool        get_in_sun(void) const {parse_data_point(); return _in_sun;}
    
    private:
        /* Disallow these */
        Generic_epsDataPoint(void) {};
        Generic_epsDataPoint(const Generic_epsDataPoint& sdp) : Sim42DataPoint(sdp) {};
        ~Generic_epsDataPoint(void) {};

        /// @name Private mutators
        inline void parse_data_point(void) const {if (_not_parsed) do_parsing();}
        void do_parsing(void) const;

        /* Specific data you need to get from the data provider to the hardware model */
        /* You only get to this data through the accessors above */
        mutable Sim42DataPoint _dp;
        int16_t _orb;
        int16_t _sc;
        mutable bool _not_parsed;
        mutable bool   _in_sun;
        mutable double _sun_vector[3];
    };
}

#endif
