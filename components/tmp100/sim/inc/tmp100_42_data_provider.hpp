#ifndef NOS3_TMP10042DATAPROVIDER_HPP
#define NOS3_TMP10042DATAPROVIDER_HPP

#include <boost/property_tree/ptree.hpp>
#include <ItcLogger/Logger.hpp>
#include <tmp100_data_point.hpp>
#include <sim_data_42socket_provider.hpp>

namespace Nos3
{
    /* Standard for a 42 data provider */
    class Tmp10042DataProvider : public SimData42SocketProvider
    {
    public:
        /* Constructors */
        Tmp10042DataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~Tmp10042DataProvider(void) {};
        Tmp10042DataProvider& operator=(const Tmp10042DataProvider&) {return *this;};

        int16_t _sc;  /* Which spacecraft number to parse out of 42 data */
    };
}

#endif
