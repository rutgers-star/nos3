#ifndef NOS3_TMP100HARDWAREMODEL_HPP
#define NOS3_TMP100HARDWAREMODEL_HPP

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
#include <tmp100_data_point.hpp>
#include <sim_i_hardware_model.hpp>


/*
** Defines
*/
#define TMP100_SIM_SUCCESS 0
#define TMP100_SIM_ERROR   1

/* TMP100 Register Addresses */
#define TMP100_REG_TEMPERATURE  0x00
#define TMP100_REG_CONFIG       0x01
#define TMP100_REG_TLOW         0x02
#define TMP100_REG_THIGH        0x03

/* TMP100 Configuration Register Bits */
#define TMP100_CONFIG_SHUTDOWN  0x01
#define TMP100_CONFIG_COMP_INT  0x02
#define TMP100_CONFIG_POL       0x04
#define TMP100_CONFIG_FAULT0    0x08
#define TMP100_CONFIG_FAULT1    0x10
#define TMP100_CONFIG_RES0      0x20
#define TMP100_CONFIG_RES1      0x40
#define TMP100_CONFIG_OS        0x80


/*
** Namespace
*/
namespace Nos3
{
    /* Forward declaration */
    class I2CSlaveConnection;

    /* Standard for a hardware model */
    class Tmp100HardwareModel : public SimIHardwareModel
    {
    public:
        /* Constructor and destructor */
        Tmp100HardwareModel(const boost::property_tree::ptree& config);
        ~Tmp100HardwareModel(void);
        std::uint8_t determine_i2c_response_for_request(const std::vector<uint8_t>& in_data, std::vector<uint8_t>& out_data);

        /* Declare I2CSlaveConnection as friend so it can access private members */
        friend class I2CSlaveConnection;

    private:
        /* Private helper methods */
        void command_callback(NosEngine::Common::Message msg); /* Handle backdoor commands and time tick to the simulator */
        int16_t temperature_to_register(double temp_celsius);
        double register_to_temperature(int16_t reg_value);

        /* Private data members */
        I2CSlaveConnection*                                 _i2c_slave_connection;

        std::string                                         _command_bus_name;
        std::unique_ptr<NosEngine::Client::Bus>             _command_bus; /* Standard */

        SimIDataProvider*                                   _tmp100_dp;

        /* Time Bus */
        std::unique_ptr<NosEngine::Client::Bus>             _time_bus;

        /* TMP100 Register Data */
        uint8_t                                             _pointer_register;
        int16_t                                             _temperature_register;
        uint8_t                                             _config_register;
        int16_t                                             _tlow_register;
        int16_t                                             _thigh_register;

        std::uint8_t                                        _enabled;
        double                                              _current_temperature;
    };

    class I2CSlaveConnection : public NosEngine::I2C::I2CSlave
    {
    public:
        I2CSlaveConnection(Tmp100HardwareModel* hm, int bus_address, std::string connection_string, std::string bus_name);
        size_t i2c_read(uint8_t *rbuf, size_t rlen);
        size_t i2c_write(const uint8_t *wbuf, size_t wlen);
    private:
        Tmp100HardwareModel* _hardware_model;
        std::uint8_t _i2c_read_valid;
        std::vector<uint8_t> _i2c_out_data;
    };

}

#endif
