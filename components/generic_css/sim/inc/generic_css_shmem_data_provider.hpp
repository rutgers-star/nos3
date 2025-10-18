#ifndef NOS3_GENERIC_CSS_SHMEM_DATA_PROVIDER_HPP
#define NOS3_GENERIC_CSS_SHMEM_DATA_PROVIDER_HPP

#include <boost/property_tree/ptree.hpp>
#include <ItcLogger/Logger.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <generic_css_data_point.hpp>
#include <sim_i_data_provider.hpp>
#include <blackboard_data.hpp>

namespace Nos3
{
    namespace bip = boost::interprocess;

    class GenericCssShmemDataProvider : public SimIDataProvider
    {
    public:
        /* Constructors */
        GenericCssShmemDataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~GenericCssShmemDataProvider(void) {};
        GenericCssShmemDataProvider& operator=(const GenericCssShmemDataProvider&) {return *this;};

        double _scale_factor; /* Corresponds to the CSS Scale Factor from 42 */
        bip::mapped_region _shm_region;
        BlackboardData*    _blackboard_data;
    };
}

#endif
