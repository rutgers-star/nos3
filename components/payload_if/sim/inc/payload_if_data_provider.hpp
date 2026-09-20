#ifndef NOS3_PAYLOAD_IFDATAPROVIDER_HPP
#define NOS3_PAYLOAD_IFDATAPROVIDER_HPP

#include <boost/property_tree/xml_parser.hpp>
#include <ItcLogger/Logger.hpp>
#include <payload_if_data_point.hpp>
#include <sim_i_data_provider.hpp>

namespace Nos3
{
    class Payload_ifDataProvider : public SimIDataProvider
    {
    public:
        /* Constructors */
        Payload_ifDataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~Payload_ifDataProvider(void) {};
        Payload_ifDataProvider& operator=(const Payload_ifDataProvider&) {return *this;};

        mutable double _request_count;
    };
}

#endif
