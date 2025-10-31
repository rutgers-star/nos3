#ifndef NOS3_HEATERHARDWAREMODEL_HPP
#define NOS3_HEATERHARDWAREMODEL_HPP

/*
** Includes
*/
#include <map>

#include <boost/tuple/tuple.hpp>
#include <boost/property_tree/ptree.hpp>

#include <Client/Bus.hpp>
#include <Client/DataNode.hpp>
#include <I2C/Client/I2CSlave.hpp>

#include <sim_i_data_provider.hpp>
#include <heater_data_point.hpp>
#include <sim_i_hardware_model.hpp>


/*
** Defines
*/
#define HEATER_SIM_SUCCESS 0
#define HEATER_SIM_ERROR   1

/* Heater Register Addresses */
#define HEATER_REG_STATUS      0x00  // Status register (power state, temperature)
#define HEATER_REG_POWER       0x01  // Power control register
#define HEATER_REG_TEMP        0x02  // Current heater temperature


/*
** Namespace
*/
namespace Nos3
{
    /* Forward declaration */
    class I2CSlaveConnection;

    /* Standard for a hardware model */
    class HeaterHardwareModel : public SimIHardwareModel
    {
    public:
        /* Constructor and destructor */
        HeaterHardwareModel(const boost::property_tree::ptree& config);
        ~HeaterHardwareModel(void);
        std::uint8_t determine_i2c_response_for_request(const std::vector<uint8_t>& in_data, std::vector<uint8_t>& out_data);

        /* Declare I2CSlaveConnection as friend so it can access private members */
        friend class I2CSlaveConnection;

    private:
        /* Private helper methods */
        void command_callback(NosEngine::Common::Message msg); /* Handle backdoor commands and time tick to the simulator */
        void update_thermal_model(void); /* Update temperature based on heater state */

        /* Private data members */
        I2CSlaveConnection*                                 _i2c_slave_connection;

        std::string                                         _command_bus_name;
        std::unique_ptr<NosEngine::Client::Bus>             _command_bus; /* Standard */

        SimIDataProvider*                                   _heater_dp;

        /* Time Bus */
        std::unique_ptr<NosEngine::Client::Bus>             _time_bus;

        /* Heater Register Data */
        uint8_t                                             _pointer_register;
        uint8_t                                             _status_register;    // Bit 0: power state, bits 1-7: reserved
        uint8_t                                             _power_register;     // 0x00=OFF, 0xAA=ON
        int16_t                                             _temperature_register; // Heater surface temperature

        std::uint8_t                                        _enabled;
        bool                                                _power_state;        // true=ON, false=OFF
        double                                              _heater_temperature; // Current heater temperature in Celsius
        double                                              _ambient_temperature; // Ambient/environment temperature

        /* Physical heater parameters (from thermal team specs) */
        double                                              _heater_power_watts;      // 31.4W
        double                                              _heater_voltage;          // 12V
        double                                              _heater_current;          // 2.62A
        double                                              _max_temperature;         // 125C
        double                                              _heating_rate;            // Degrees C per second when ON
        double                                              _cooling_rate;            // Degrees C per second when OFF
    };

    class I2CSlaveConnection : public NosEngine::I2C::I2CSlave
    {
    public:
        I2CSlaveConnection(HeaterHardwareModel* hm, int bus_address, std::string connection_string, std::string bus_name);
        size_t i2c_read(uint8_t *rbuf, size_t rlen);
        size_t i2c_write(const uint8_t *wbuf, size_t wlen);
    private:
        HeaterHardwareModel* _hardware_model;
        std::uint8_t _i2c_read_valid;
        std::vector<uint8_t> _i2c_out_data;
    };

}

#endif
