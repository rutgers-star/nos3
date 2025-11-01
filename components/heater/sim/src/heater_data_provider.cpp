#include <ItcLogger/Logger.hpp>

#include <heater_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(HeaterDataProvider,"HEATER_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    HeaterDataProvider::HeaterDataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        _request_count = 0.0;
        sim_logger->trace("HeaterDataProvider::HeaterDataProvider:  Configured data provider.");
    }

    boost::shared_ptr<SimIDataPoint> HeaterDataProvider::get_data_point(void) const
    {
        sim_logger->trace("HeaterDataProvider::get_data_point:  Executed");

        /* Prepare the provider data */
        _request_count += 1.0;
        SimIDataPoint *dp = new HeaterDataPoint(_request_count);

        return boost::shared_ptr<SimIDataPoint>(dp);
    }
}
