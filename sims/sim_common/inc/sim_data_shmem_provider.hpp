/* Copyright (C) 2015 - 2025 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

#ifndef NOS3_SIMDATASHMEMPROVIDER_HPP
#define NOS3_SIMDATASHMEMPROVIDER_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <sim_i_data_provider.hpp>
#include <sim_shmem_data_point.hpp>

#include <blackboard_data.hpp>

namespace Nos3
{
    namespace bip = boost::interprocess;

    /** \brief Class for a provider of simulation data that provides data from a shared memory connection.
     */

    class SimDataShmemProvider : public SimIDataProvider
    {
    public:
        /// @name Constructors / destructors
        //@{
        /// \brief Constructor taking a configuration object.
        /// @param  sc  The configuration for the simulation
        SimDataShmemProvider(const boost::property_tree::ptree& config);
        ~SimDataShmemProvider(void) {}
        //@}

        /// @name Non-mutating public worker methods
        //@{
        /** \brief Method to retrieve simulation data.
         *
         * @returns                     A data point of simulation data.
         */
        virtual boost::shared_ptr<SimIDataPoint> get_data_point(void) const
        {
            boost::shared_ptr<SimShmemDataPoint> dp;
            {
                bip::scoped_lock<bip::interprocess_mutex> lock(_blackboard_data->mutex);
                dp = boost::shared_ptr<SimShmemDataPoint>(
                    new SimShmemDataPoint(_blackboard_data->svb, _blackboard_data->bvb, _blackboard_data->Hvb, 
                                          _blackboard_data->GyroRate, _blackboard_data->CSSValid, _blackboard_data->CSSIllum, 
                                          _blackboard_data->FSSValid, _blackboard_data->FSSSunAng, _blackboard_data->STValid,
                                          _blackboard_data->STqn, _blackboard_data->GPSPosN, _blackboard_data->GPSVelN, 
                                          _blackboard_data->AccelAcc, _blackboard_data->WhlH));
                // lock is released when scope ends
            }
            return dp;
        }

    private:
        bip::mapped_region _shm_region;
        BlackboardData*    _blackboard_data;
    };
}

#endif
