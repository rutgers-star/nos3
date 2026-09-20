#ifndef NOS3_RASPBERRY_PI42DATAPROVIDER_HPP
#define NOS3_RASPBERRY_PI42DATAPROVIDER_HPP

#include <boost/property_tree/ptree.hpp>
#include <ItcLogger/Logger.hpp>
#include <raspberry_pi_data_point.hpp>
#include <sim_data_42socket_provider.hpp>

namespace Nos3
{
    /* Standard for a 42 data provider */
    class Raspberry_pi42DataProvider : public SimData42SocketProvider
    {
    public:
        /* Constructors */
        Raspberry_pi42DataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~Raspberry_pi42DataProvider(void) {};
        Raspberry_pi42DataProvider& operator=(const Raspberry_pi42DataProvider&) {return *this;};

        int16_t _sc;  /* Which spacecraft number to parse out of 42 data */
    };
}

#endif
