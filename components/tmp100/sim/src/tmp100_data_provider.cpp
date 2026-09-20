#include <tmp100_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(Tmp100DataProvider,"TMP100_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    Tmp100DataProvider::Tmp100DataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        sim_logger->trace("Tmp100DataProvider::Tmp100DataProvider:  Constructor executed");
        _request_count = 0;
    }

    boost::shared_ptr<SimIDataPoint> Tmp100DataProvider::get_data_point(void) const
    {
        sim_logger->trace("Tmp100DataProvider::get_data_point:  Executed");

        /* Prepare the provider data */
        _request_count++;

        /* Request a data point */
        SimIDataPoint *dp = new Tmp100DataPoint(_request_count);

        /* Return the data point */
        return boost::shared_ptr<SimIDataPoint>(dp);
    }
}
