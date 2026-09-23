#include <payload_if_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(Payload_ifDataProvider,"PAYLOAD_IF_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    Payload_ifDataProvider::Payload_ifDataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        sim_logger->trace("Payload_ifDataProvider::Payload_ifDataProvider:  Constructor executed");
        _request_count = 0;
    }

    boost::shared_ptr<SimIDataPoint> Payload_ifDataProvider::get_data_point(void) const
    {
        sim_logger->trace("Payload_ifDataProvider::get_data_point:  Executed");

        /* Prepare the provider data */
        _request_count++;

        /* Request a data point */
        SimIDataPoint *dp = new Payload_ifDataPoint(_request_count);

        /* Return the data point */
        return boost::shared_ptr<SimIDataPoint>(dp);
    }
}
