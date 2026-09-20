#ifndef NOS3_HEATERDATAPROVIDER_HPP
#define NOS3_HEATERDATAPROVIDER_HPP

#include <boost/property_tree/ptree.hpp>

#include <sim_i_data_provider.hpp>
#include <heater_data_point.hpp>

namespace Nos3
{
    class HeaterDataProvider : public SimIDataProvider
    {
    public:
        HeaterDataProvider(const boost::property_tree::ptree& config);
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        mutable double _request_count;
    };
}

#endif
