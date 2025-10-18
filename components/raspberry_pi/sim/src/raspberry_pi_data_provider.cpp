#include <raspberry_pi_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(Raspberry_piDataProvider,"RASPBERRY_PI_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    Raspberry_piDataProvider::Raspberry_piDataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        sim_logger->trace("Raspberry_piDataProvider::Raspberry_piDataProvider:  Constructor executed");
        _request_count = 0;
    }

    boost::shared_ptr<SimIDataPoint> Raspberry_piDataProvider::get_data_point(void) const
    {
        sim_logger->trace("Raspberry_piDataProvider::get_data_point:  Executed");

        /* Prepare the provider data */
        _request_count++;

        /* Request a data point */
        SimIDataPoint *dp = new Raspberry_piDataPoint(_request_count);

        /* Return the data point */
        return boost::shared_ptr<SimIDataPoint>(dp);
    }
}
