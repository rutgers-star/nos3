#include <tmp100_hardware_model.hpp>

namespace Nos3
{
    REGISTER_HARDWARE_MODEL(Tmp100HardwareModel,"TMP100");

    extern ItcLogger::Logger *sim_logger;

    Tmp100HardwareModel::Tmp100HardwareModel(const boost::property_tree::ptree& config) : SimIHardwareModel(config),
    _pointer_register(TMP100_REG_TEMPERATURE), _temperature_register(0), _config_register(0x60),
    _tlow_register(0), _thigh_register(0), _enabled(TMP100_SIM_SUCCESS), _current_temperature(25.0)
    {
        /* Get the NOS engine connection string */
        std::string connection_string = config.get("common.nos-connection-string", "tcp://127.0.0.1:12001");
        sim_logger->info("Tmp100HardwareModel::Tmp100HardwareModel:  NOS Engine connection string: %s.", connection_string.c_str());

        // Set up the time node which is **required** for this model
        std::string time_bus_name = "command";
        if (config.get_child_optional("hardware-model.connections")) {
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("hardware-model.connections")) {
                if (v.second.get("type", "").compare("time") == 0) {
                    time_bus_name = v.second.get("bus-name", "command");
                    break;
                }
            }
        }
        _time_bus.reset(new NosEngine::Client::Bus(_hub, connection_string, time_bus_name));
        sim_logger->debug("Tmp100HardwareModel::Tmp100HardwareModel:  Time bus %s now active.", time_bus_name.c_str());

        /* Get a data provider */
        std::string dp_name = config.get("simulator.hardware-model.data-provider.type", "TMP100_PROVIDER");
        _tmp100_dp = SimDataProviderFactory::Instance().Create(dp_name, config);
        sim_logger->info("Tmp100HardwareModel::Tmp100HardwareModel:  Data provider %s created.", dp_name.c_str());

        /* Get on I2C protocol bus */
        std::string bus_name = "i2c_3";
        int bus_address = 0x48;
        if (config.get_child_optional("simulator.hardware-model.connections"))
        {
            /* Loop through the connections for hardware model */
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("simulator.hardware-model.connections"))
            {
                if (v.second.get("type", "").compare("i2c") == 0)
                {
                    bus_name = v.second.get("bus-name", bus_name);
                    bus_address = v.second.get("address", bus_address);
                    break;
                }
            }
        }

        _i2c_slave_connection = new I2CSlaveConnection(this, bus_address, connection_string, bus_name);
        sim_logger->info("Tmp100HardwareModel::Tmp100HardwareModel:  Now on I2C bus name %s as address 0x%02x.", bus_name.c_str(), bus_address);

        /* Get on the command bus */
        _command_bus_name = "command";
        if (config.get_child_optional("hardware-model.connections"))
        {
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("hardware-model.connections"))
            {
                if (v.second.get("type", "").compare("time") == 0)
                {
                    _command_bus_name = v.second.get("bus-name", "command");
                    break;
                }
            }
        }
        _command_bus.reset(new NosEngine::Client::Bus(_hub, connection_string, _command_bus_name));
        sim_logger->info("Tmp100HardwareModel::Tmp100HardwareModel:  Now on time bus named %s.", _command_bus_name.c_str());

        /* Initialize TMP100 registers with default values */
        _temperature_register = temperature_to_register(_current_temperature);
        _tlow_register = temperature_to_register(75.0);   // Default TLOW = 75°C
        _thigh_register = temperature_to_register(80.0);  // Default THIGH = 80°C

        sim_logger->info("Tmp100HardwareModel::Tmp100HardwareModel:  Initial temperature = %.2f°C (register: 0x%04x)",
            _current_temperature, _temperature_register);

        /* Construction complete */
        sim_logger->info("Tmp100HardwareModel::Tmp100HardwareModel:  Construction complete.");
    }


    Tmp100HardwareModel::~Tmp100HardwareModel(void)
    {
        /* Close the protocol bus */
        delete _i2c_slave_connection;
        _i2c_slave_connection = nullptr;

        /* Clean up the data provider */
        delete _tmp100_dp;
        _tmp100_dp = nullptr;

        /* The bus will clean up the time and sim nodes */
    }


    /* Automagically set up by the base class to be called */
    void Tmp100HardwareModel::command_callback(NosEngine::Common::Message msg)
    {
        /* Get the data out of the message */
        NosEngine::Common::DataBufferOverlay dbf(const_cast<NosEngine::Utility::Buffer&>(msg.buffer));
        sim_logger->info("Tmp100HardwareModel::command_callback:  Received command: %s.", dbf.data);

        /* Do something with the data */
        std::string command = dbf.data;
        std::string response = "Tmp100HardwareModel::command_callback:  INVALID COMMAND! (Try HELP)";
        boost::to_upper(command);
        if (command.compare("HELP") == 0)
        {
            response = "Tmp100HardwareModel::command_callback: Valid commands are HELP, ENABLE, DISABLE, TEMPERATURE=X, or STOP";
        }
        else if (command.compare("ENABLE") == 0)
        {
            _enabled = TMP100_SIM_SUCCESS;
            response = "Tmp100HardwareModel::command_callback:  Enabled";
        }
        else if (command.compare("DISABLE") == 0)
        {
            _enabled = TMP100_SIM_ERROR;
            response = "Tmp100HardwareModel::command_callback:  Disabled";
        }
        else if (command.substr(0,12).compare("TEMPERATURE=") == 0)
        {
            try
            {
                /* Temperature update from heater or manual command */
                _current_temperature = std::stod(command.substr(12));
                _temperature_register = temperature_to_register(_current_temperature);
                response = "Tmp100HardwareModel::command_callback:  Temperature set to " + std::to_string(_current_temperature) + "°C";

                /* If this is from heater, don't send reply (non-confirmed message) */
                if (msg.is_reply_expected()) {
                    sim_logger->info("Tmp100HardwareModel::command_callback:  Manual temperature update");
                } else {
                    sim_logger->debug("Tmp100HardwareModel::command_callback:  Temperature updated from heater: %.2f°C", _current_temperature);
                    return; /* Don't send reply for heater updates */
                }
            }
            catch (...)
            {
                response = "Tmp100HardwareModel::command_callback:  Temperature invalid";
            }
        }
        else if (command.compare("STOP") == 0)
        {
            _keep_running = false;
            response = "Tmp100HardwareModel::command_callback:  Stopping";
        }

        /* Send a reply */
        sim_logger->info("Tmp100HardwareModel::command_callback:  Sending reply: %s.", response.c_str());
        _command_node->send_reply_message_async(msg, response.size(), response.c_str());
    }

    /* Convert temperature in Celsius to TMP100 register value */
    int16_t Tmp100HardwareModel::temperature_to_register(double temp_celsius)
    {
        /* TMP100 uses 12-bit resolution with 0.0625°C per LSB
         * Temperature is stored in upper 12 bits of 16-bit register
         * Formula: register_value = (temp / 0.0625) << 4
         */
        int16_t adc_code = static_cast<int16_t>(temp_celsius / 0.0625);
        return (adc_code << 4);
    }

    /* Convert TMP100 register value to temperature in Celsius */
    double Tmp100HardwareModel::register_to_temperature(int16_t reg_value)
    {
        /* Extract 12-bit temperature value from upper bits */
        int16_t adc_code = reg_value >> 4;
        return adc_code * 0.0625;
    }

    /* Protocol callback - Handle I2C write transactions (pointer + optional data) */
    std::uint8_t Tmp100HardwareModel::determine_i2c_response_for_request(const std::vector<uint8_t>& in_data, std::vector<uint8_t>& /* out_data */)
    {
        std::uint8_t valid = TMP100_SIM_SUCCESS;

        /* Retrieve data and log in man readable format */
        sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  WRITE %s",
            SimIHardwareModel::uint8_vector_to_hex_string(in_data).c_str());

        /* Check simulator is enabled */
        if (_enabled != TMP100_SIM_SUCCESS)
        {
            sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  TMP100 sim disabled!");
            return TMP100_SIM_ERROR;
        }

        /* Process write transaction based on length */
        if (in_data.size() == 1)
        {
            /* Write pointer register only */
            _pointer_register = in_data[0] & 0x03;  // Only 2 bits used for pointer
            sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  Pointer register set to 0x%02x", _pointer_register);
        }
        else if (in_data.size() == 2)
        {
            /* Write pointer + 1 byte (config register) */
            _pointer_register = in_data[0] & 0x03;

            if (_pointer_register == TMP100_REG_CONFIG)
            {
                _config_register = in_data[1];
                sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  Config register set to 0x%02x", _config_register);
            }
            else
            {
                sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  Invalid 1-byte write to register 0x%02x", _pointer_register);
                valid = TMP100_SIM_ERROR;
            }
        }
        else if (in_data.size() == 3)
        {
            /* Write pointer + 2 bytes (TLOW or THIGH) */
            _pointer_register = in_data[0] & 0x03;
            int16_t value = (in_data[1] << 8) | in_data[2];

            if (_pointer_register == TMP100_REG_TLOW)
            {
                _tlow_register = value;
                sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  TLOW set to 0x%04x (%.2f°C)",
                    _tlow_register, register_to_temperature(_tlow_register));
            }
            else if (_pointer_register == TMP100_REG_THIGH)
            {
                _thigh_register = value;
                sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  THIGH set to 0x%04x (%.2f°C)",
                    _thigh_register, register_to_temperature(_thigh_register));
            }
            else
            {
                sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  Invalid 2-byte write to register 0x%02x", _pointer_register);
                valid = TMP100_SIM_ERROR;
            }
        }
        else
        {
            sim_logger->debug("Tmp100HardwareModel::determine_i2c_response_for_request:  Invalid write size of %ld received!", in_data.size());
            valid = TMP100_SIM_ERROR;
        }

        return valid;
    }

    I2CSlaveConnection::I2CSlaveConnection(Tmp100HardwareModel* hm,
        int bus_address, std::string connection_string, std::string bus_name)
        : NosEngine::I2C::I2CSlave(bus_address, connection_string, bus_name)
    {
        _hardware_model = hm;
        _i2c_read_valid = TMP100_SIM_SUCCESS;
    }

    size_t I2CSlaveConnection::i2c_read(uint8_t *rbuf, size_t rlen)
    {
        /* Generate read response based on current pointer register */
        _i2c_out_data.clear();
        _i2c_out_data.resize(2);  // TMP100 always returns 2 bytes

        /* Update temperature from data provider if reading temperature register */
        if (_hardware_model->_pointer_register == TMP100_REG_TEMPERATURE)
        {
            if (_hardware_model->_tmp100_dp != nullptr)
            {
                boost::shared_ptr<Tmp100DataPoint> data_point = boost::dynamic_pointer_cast<Tmp100DataPoint>(_hardware_model->_tmp100_dp->get_data_point());
                if (data_point)
                {
                    _hardware_model->_current_temperature = data_point->get_tmp100_data_x(); // Using X axis as temperature
                    _hardware_model->_temperature_register = _hardware_model->temperature_to_register(_hardware_model->_current_temperature);
                }
            }
        }

        /* Prepare response based on pointer register */
        switch (_hardware_model->_pointer_register)
        {
            case TMP100_REG_TEMPERATURE:
                _i2c_out_data[0] = (_hardware_model->_temperature_register >> 8) & 0xFF;
                _i2c_out_data[1] = _hardware_model->_temperature_register & 0xFF;
                sim_logger->debug("i2c_read TEMP: 0x%04x (%.2f°C)",
                    _hardware_model->_temperature_register, _hardware_model->_current_temperature);
                break;

            case TMP100_REG_CONFIG:
                _i2c_out_data[0] = _hardware_model->_config_register;
                _i2c_out_data[1] = 0x00;  // Pad with 0 (config is 1 byte)
                sim_logger->debug("i2c_read CONFIG: 0x%02x", _hardware_model->_config_register);
                break;

            case TMP100_REG_TLOW:
                _i2c_out_data[0] = (_hardware_model->_tlow_register >> 8) & 0xFF;
                _i2c_out_data[1] = _hardware_model->_tlow_register & 0xFF;
                sim_logger->debug("i2c_read TLOW: 0x%04x (%.2f°C)",
                    _hardware_model->_tlow_register, _hardware_model->register_to_temperature(_hardware_model->_tlow_register));
                break;

            case TMP100_REG_THIGH:
                _i2c_out_data[0] = (_hardware_model->_thigh_register >> 8) & 0xFF;
                _i2c_out_data[1] = _hardware_model->_thigh_register & 0xFF;
                sim_logger->debug("i2c_read THIGH: 0x%04x (%.2f°C)",
                    _hardware_model->_thigh_register, _hardware_model->register_to_temperature(_hardware_model->_thigh_register));
                break;

            default:
                _i2c_out_data[0] = 0x00;
                _i2c_out_data[1] = 0x00;
                sim_logger->debug("i2c_read: Invalid pointer register 0x%02x", _hardware_model->_pointer_register);
                break;
        }

        /* Copy to read buffer */
        size_t num_read;
        for(num_read = 0; num_read < rlen && num_read < _i2c_out_data.size(); num_read++)
        {
            rbuf[num_read] = _i2c_out_data[num_read];
        }
        sim_logger->debug("i2c_read[%ld]: %s", num_read, SimIHardwareModel::uint8_vector_to_hex_string(_i2c_out_data).c_str());

        return num_read;
    }

    size_t I2CSlaveConnection::i2c_write(const uint8_t *wbuf, size_t wlen)
    {
        std::vector<uint8_t> in_data(wbuf, wbuf + wlen);
        sim_logger->debug("i2c_write[%ld]: %s", wlen,
            SimIHardwareModel::uint8_vector_to_hex_string(in_data).c_str());

        // Process the write (sets pointer and/or writes register data)
        _i2c_read_valid = _hardware_model->determine_i2c_response_for_request(in_data, _i2c_out_data);

        return wlen;
    }
}
