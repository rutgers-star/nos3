#ifndef NOS3_RASPBERRY_PIDATAPROVIDER_HPP
#define NOS3_RASPBERRY_PIDATAPROVIDER_HPP

#include <boost/property_tree/xml_parser.hpp>
#include <ItcLogger/Logger.hpp>
#include <raspberry_pi_data_point.hpp>
#include <sim_i_data_provider.hpp>

namespace Nos3
{
    class Raspberry_piDataProvider : public SimIDataProvider
    {
    public:
        /* Constructors */
        Raspberry_piDataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~Raspberry_piDataProvider(void) {};
        Raspberry_piDataProvider& operator=(const Raspberry_piDataProvider&) {return *this;};

        mutable double _request_count;
    };
}

#endif
