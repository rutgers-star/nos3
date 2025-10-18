#ifndef NOS3_GENERIC_MAGDATAPROVIDER_HPP
#define NOS3_GENERICMAGDATAPROVIDER_HPP

#include <boost/property_tree/xml_parser.hpp>
#include <ItcLogger/Logger.hpp>
#include <generic_mag_data_point.hpp>
#include <sim_i_data_provider.hpp>

namespace Nos3
{
    class Generic_magDataProvider : public SimIDataProvider
    {
    public:
        /* Constructors */
        Generic_magDataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~Generic_magDataProvider(void) {};
        Generic_magDataProvider& operator=(const Generic_magDataProvider&) {return *this;};

        mutable double _request_count;
    };
}

#endif
