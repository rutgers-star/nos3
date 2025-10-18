#include <signal.h>
#include <sstream>

#include <boost/property_tree/json_parser.hpp>

#include <sim_cmdbus_bridge.hpp>

namespace Nos3
{
    REGISTER_HARDWARE_MODEL(SimCmdBusBridge,"SIM_CMDBUS_BRIDGE");

    ItcLogger::Logger *sim_logger;

    SimCmdBusBridge::SimCmdBusBridge(const boost::property_tree::ptree& config)
    :   SimIHardwareModel(config),
        _msg_svr(
            config.get("simulator.hardware-model.server-PORT", 12020))
    {
        if (! _msg_svr.init())
        {
            throw std::runtime_error("Command bus bridge server failed to initialize");
        }
    }

    SimCmdBusBridge::~SimCmdBusBridge()
    {}

    void SimCmdBusBridge::run(void)
    {
        while (_keep_running.load())
        {
            /* This will block until
             * 1) New client connection
             * 2) New message from connected client
             * 3) OS signal interrupts the program
            **/
            bool data_available = _msg_svr.listen_for_data();

            if (data_available)
            {
                std::string msg;
                while (_msg_svr.get_next_message(msg))
                {
                    process_msg(msg);
                }
            }
        }
    }

    void SimCmdBusBridge::process_msg(std::string &msg)
    {
        /**
          * Try to parse the string as a JSON message.  If this fails
          * then don't process it.
        **/
        try
        {
            std::stringstream stream(msg);
            boost::property_tree::ptree pt;
            std::string node_name = "";
            std::string cmd = "";

            boost::property_tree::read_json(stream, pt);

            /**
              * Determine if the node name specified in the message exists
              * on the command bus.  If it does, send the message.
            **/
            try
            {
                node_name = pt.get<std::string>("node");
                cmd = pt.get<std::string>("cmd");

                sim_logger->info("Received new message destined for %s with command %s",
                    node_name.c_str(), cmd.c_str());
                
                // Add 1 since C++ string size does not include null termination character.
                // We want this character sent so the buffer on the receive side is
                // interpreted as a valid C string.
                _command_node->send_non_confirmed_message_async(node_name, cmd.size()+1, cmd.c_str());;
            }
            catch(const std::exception& e)
            {
                sim_logger->error("Unable to send message to %s: %s", node_name.c_str(), e.what());
            }
            catch(...)
            {
                sim_logger->error("Unable to send message to %s: unspecified error", node_name.c_str());
            }
        }
        catch(const std::exception& parse_ex)
        {
            sim_logger->error("Could not parse message '%s' as JSON: %s", msg.c_str(), parse_ex.what());
        }
    }
}

//==============================================================================
// Main
//==============================================================================

Nos3::SimConfig* sim_cfg;

void signal_handler(int signum)
{
    signum = signum; // TODO: unused-parameter
    sim_cfg->stop_simulator();
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    std::string simulator_name = "cmdbus-bridge";

    // Determine the configuration and run the simulator
    sim_cfg = new Nos3::SimConfig(argc, argv);

    Nos3::sim_logger->info("main:  %s simulator starting", simulator_name.c_str());

    try
    {
        sim_cfg->run_simulator(simulator_name);
    }
    catch(const std::exception& e)
    {
        Nos3::sim_logger->error("main: exception caught: %s", e.what());
    }
    catch(...)
    {
        Nos3::sim_logger->error("Unspecified exception\n");
    }

    delete sim_cfg;
    Nos3::sim_logger->info("main:  %s simulator terminating", simulator_name.c_str());
}
