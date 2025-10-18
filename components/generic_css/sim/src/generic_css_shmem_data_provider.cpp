#include <generic_css_shmem_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(GenericCssShmemDataProvider,"GENERIC_CSS_SHMEM_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    GenericCssShmemDataProvider::GenericCssShmemDataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        sim_logger->trace("GenericCssShmemDataProvider::GenericCssShmemDataProvider:  Constructor executed");
        const std::string shm_name = config.get("simulator.hardware-model.data-provider.shared-memory-name", "Blackboard");
        const size_t shm_size = sizeof(BlackboardData);
        bip::shared_memory_object shm(bip::open_or_create, shm_name.c_str(), bip::read_write);
        shm.truncate(shm_size);
        bip::mapped_region shm_region(shm, bip::read_write);
        _shm_region = std::move(shm_region); // don't let this go out of scope/get destroyed
        _blackboard_data = static_cast<BlackboardData*>(_shm_region.get_address());    
        _scale_factor = config.get("simulator.hardware-model.data-provider.42-css-scale-factor", 1.0);
    }

    boost::shared_ptr<SimIDataPoint> GenericCssShmemDataProvider::get_data_point(void) const
    {
        boost::shared_ptr<Generic_cssDataPoint> dp;
        {
            bip::scoped_lock<bip::interprocess_mutex> lock(_blackboard_data->mutex);
            if (_blackboard_data->CSSValid[0] == 0) _blackboard_data->CSSIllum[0] = 0;
            if (_blackboard_data->CSSValid[1] == 0) _blackboard_data->CSSIllum[1] = 0;
            if (_blackboard_data->CSSValid[2] == 0) _blackboard_data->CSSIllum[2] = 0;
            if (_blackboard_data->CSSValid[3] == 0) _blackboard_data->CSSIllum[3] = 0;
            if (_blackboard_data->CSSValid[4] == 0) _blackboard_data->CSSIllum[4] = 0;
            if (_blackboard_data->CSSValid[5] == 0) _blackboard_data->CSSIllum[5] = 0;
            dp = boost::shared_ptr<Generic_cssDataPoint>(
                new Generic_cssDataPoint(_scale_factor, _blackboard_data->CSSIllum[0], _blackboard_data->CSSIllum[1], _blackboard_data->CSSIllum[2], 
                    _blackboard_data->CSSIllum[3], _blackboard_data->CSSIllum[4], _blackboard_data->CSSIllum[5]));
            // lock is released when scope ends
        }
        std::vector<float> values = dp->getValues();
        sim_logger->debug("GenericCssShmemDataProvider::get_data_point: values=%f, %f, %f, %f, %f, %f",
            values[0], values[1], values[2], values[3], values[4], values[5]);
        return dp;
    }
}
