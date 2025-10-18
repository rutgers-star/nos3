#include <generic_fss_shmem_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(GenericFssShmemDataProvider,"GENERIC_FSS_SHMEM_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    GenericFssShmemDataProvider::GenericFssShmemDataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        sim_logger->trace("GenericFssShmemDataProvider::GenericFssShmemDataProvider:  Constructor executed");
        const std::string shm_name = config.get("simulator.hardware-model.data-provider.shared-memory-name", "Blackboard");
        const size_t shm_size = sizeof(BlackboardData);
        bip::shared_memory_object shm(bip::open_or_create, shm_name.c_str(), bip::read_write);
        shm.truncate(shm_size);
        bip::mapped_region shm_region(shm, bip::read_write);
        _shm_region = std::move(shm_region); // don't let this go out of scope/get destroyed
        _blackboard_data = static_cast<BlackboardData*>(_shm_region.get_address());    
    }

    boost::shared_ptr<SimIDataPoint> GenericFssShmemDataProvider::get_data_point(void) const
    {
        boost::shared_ptr<Generic_fssDataPoint> dp;
        {
            bip::scoped_lock<bip::interprocess_mutex> lock(_blackboard_data->mutex);
            dp = boost::shared_ptr<Generic_fssDataPoint>(
                new Generic_fssDataPoint(_blackboard_data->FSSValid, _blackboard_data->FSSSunAng[0], _blackboard_data->FSSSunAng[1]));
            // lock is released when scope ends
        }
        sim_logger->debug("GenericFssShmemDataProvider::get_data_point: valid=%d, alpha=%f, beta=%f", 
            dp->get_generic_fss_valid(), dp->get_generic_fss_alpha(), dp->get_generic_fss_beta());
        return dp;
    }
}
