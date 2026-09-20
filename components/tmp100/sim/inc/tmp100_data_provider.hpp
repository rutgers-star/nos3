#ifndef NOS3_TMP100DATAPROVIDER_HPP
#define NOS3_TMP100DATAPROVIDER_HPP

#include <boost/property_tree/xml_parser.hpp>
#include <ItcLogger/Logger.hpp>
#include <tmp100_data_point.hpp>
#include <sim_i_data_provider.hpp>

namespace Nos3
{
    class Tmp100DataProvider : public SimIDataProvider
    {
    public:
        /* Constructors */
        Tmp100DataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~Tmp100DataProvider(void) {};
        Tmp100DataProvider& operator=(const Tmp100DataProvider&) {return *this;};

        mutable double _request_count;
    };
}

#endif
