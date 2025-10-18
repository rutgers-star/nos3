/* Copyright (C) 2015 - 2017 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

   This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including, but not
   limited to, any warranty that the software will conform to, specifications any implied warranties of merchantability, fitness
   for a particular purpose, and freedom from infringement, and any warranty that the documentation will conform to the program, or
   any warranty that the software will be error free.

   In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or consequential damages,
   arising out of, resulting from, or in any way connected with the software or its documentation.  Whether or not based upon warranty,
   contract, tort or otherwise, and whether or not loss was sustained from, or arose out of the results of, or use of, the software,
   documentation or services provided hereunder

   ITC Team
   NASA IV&V
   ivv-itc@lists.nasa.gov
*/

#include <sim_data_shmem_provider.hpp>

#include <ItcLogger/Logger.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(SimDataShmemProvider,"SHMEMPROVIDER");

    extern ItcLogger::Logger *sim_logger;

    /*************************************************************************
     * Constructors / Destructors
     *************************************************************************/
    SimDataShmemProvider::SimDataShmemProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        const std::string shm_name = config.get("simulator.hardware-model.shared-memory-name", "Blackboard");
        const size_t shm_size = sizeof(BlackboardData);
        bip::shared_memory_object shm(bip::open_or_create, shm_name.c_str(), bip::read_write);
        shm.truncate(shm_size);
        bip::mapped_region shm_region(shm, bip::read_write);
        _shm_region = std::move(shm_region); // don't let this go out of scope/get destroyed
        _blackboard_data = static_cast<BlackboardData*>(_shm_region.get_address());    
    }
}
