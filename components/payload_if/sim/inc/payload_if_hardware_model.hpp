#ifndef NOS3_PAYLOAD_IFHARDWAREMODEL_HPP
#define NOS3_PAYLOAD_IFHARDWAREMODEL_HPP

/*
** Includes
*/
#include <mutex>

#include <boost/property_tree/ptree.hpp>

#include <Client/Bus.hpp>
#include <Uart/Client/Uart.hpp>

#include <sim_i_hardware_model.hpp>

#include <payobc_link_model.hpp>


/*
** Namespace
*/
namespace Nos3
{
    /*
    ** Simulated PayOBC on the BusOBC payload UART. PayObcLinkModel holds the
    ** protocol behavior; this class connects it to NOS Engine: UART bytes in,
    ** paced UART writes out on each time tick, and simulator-control commands
    ** for fault injection.
    */
    class Payload_ifHardwareModel : public SimIHardwareModel
    {
    public:
        /* Constructor and destructor */
        Payload_ifHardwareModel(const boost::property_tree::ptree& config);
        ~Payload_ifHardwareModel(void);

    private:
        /* Private helper methods */
        void uart_read_callback(const uint8_t *buf, size_t len); /* Bytes from the BusOBC */
        void time_tick_callback(NosEngine::Common::SimTime time); /* Paced UART transmit */
        void command_callback(NosEngine::Common::Message msg); /* Simulator control and fault injection */
        std::string process_command(const std::string& command);
        std::string counters_string(void) const;

        /* Private data members */
        std::unique_ptr<NosEngine::Uart::Uart>              _uart_connection;
        std::unique_ptr<NosEngine::Client::Bus>             _time_bus;

        /* Guards everything below; UART, tick, and command callbacks run on different threads */
        mutable std::mutex                                  _mutex;
        PayObcLinkModel                                     _model;
        bool                                                _enabled;
        std::uint16_t                                       _seed;
        double                                              _tx_bytes_per_tick; /* UART line rate per time tick */
        double                                              _tx_credit;         /* Bytes the UART may accept now */
    };
}

#endif
