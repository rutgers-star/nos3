/* Copyright (C) 2009 - 2021 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

/* Standard Includes */
#include <string>

#include <boost/property_tree/ptree.hpp>

#include <Client/Bus.hpp>

#include <sim_i_hardware_model.hpp>

namespace Nos3
{
    /** \brief Class to drive NOS time.
     *
     */
    class TimeDriver : public SimIHardwareModel
    {
    public:
        /*************************************************************************
         * Constructors / destructors
         *************************************************************************/

        TimeDriver(const boost::property_tree::ptree& config);

        /*************************************************************************
         * Mutating public worker methods
         *************************************************************************/

        void run(void);

    private:
        void update_display(void);
        void update_time_busses(void);
        double time_diff(void);
        void command_callback(NosEngine::Common::Message msg); /* Handle backdoor commands to the simulator */

        // Private data
        const bool                                     _active;
        unsigned int                                   _time_counter;
        unsigned int                                   _display_counter;
        struct timeval                                 _now, _then;
        unsigned int                                   _pause_ticks;
        double                                         _last_time_diff;

        struct TimeBusInfo {
            std::string                                 time_uri;
            std::string                                 time_bus_name;
            std::unique_ptr<NosEngine::Client::Bus>     time_bus;
        };
        std::vector<TimeBusInfo>                        _time_bus_info;

        std::string                                     _command_bus_name;
        std::unique_ptr<NosEngine::Client::Bus>         _command_bus; /* Standard */
    };
}
