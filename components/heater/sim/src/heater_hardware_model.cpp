#include <heater_hardware_model.hpp>

namespace Nos3
{
    REGISTER_HARDWARE_MODEL(HeaterHardwareModel,"HEATER");

    extern ItcLogger::Logger *sim_logger;

    HeaterHardwareModel::HeaterHardwareModel(const boost::property_tree::ptree& config) : SimIHardwareModel(config),
    _pointer_register(HEATER_REG_STATUS), _status_register(0), _power_register(0x00),
    _temperature_register(0), _enabled(HEATER_SIM_SUCCESS), _power_state(false),
    _heater_temperature(25.0), _ambient_temperature(20.0)
    {
        /* Get the NOS engine connection string */
        std::string connection_string = config.get("common.nos-connection-string", "tcp://127.0.0.1:12001");
        sim_logger->info("HeaterHardwareModel::HeaterHardwareModel:  NOS Engine connection string: %s.", connection_string.c_str());

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
        sim_logger->debug("HeaterHardwareModel::HeaterHardwareModel:  Time bus %s now active.", time_bus_name.c_str());

        /* Get a data provider */
        std::string dp_name = config.get("simulator.hardware-model.data-provider.type", "HEATER_PROVIDER");
        _heater_dp = SimDataProviderFactory::Instance().Create(dp_name, config);
        sim_logger->info("HeaterHardwareModel::HeaterHardwareModel:  Data provider %s created.", dp_name.c_str());

        /* Get heater physical parameters from XML config */
        _heater_power_watts = atof(config.get("simulator.hardware-model.physical.power-watts", "31.4").c_str());
        _heater_voltage = atof(config.get("simulator.hardware-model.physical.voltage", "12.0").c_str());
        _heater_current = atof(config.get("simulator.hardware-model.physical.current", "2.62").c_str());
        _max_temperature = atof(config.get("simulator.hardware-model.physical.max-temperature", "125.0").c_str());
        _heating_rate = atof(config.get("simulator.hardware-model.physical.heating-rate-celsius-per-sec", "0.0333").c_str()); // 2°C/min
        _cooling_rate = atof(config.get("simulator.hardware-model.physical.cooling-rate-celsius-per-sec", "0.00833").c_str()); // 0.5°C/min

        sim_logger->info("HeaterHardwareModel::HeaterHardwareModel:  Heater specs: %.1fW, %.1fV, %.2fA, heating rate: %.4f°C/s",
            _heater_power_watts, _heater_voltage, _heater_current, _heating_rate);

        /* Get on I2C protocol bus */
        std::string bus_name = "i2c_3";
        int bus_address = 0x49;
        if (config.get_child_optional("simulator.hardware-model.connections"))
        {
            /* Loop through the connections for hardware model */
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("simulator.hardware-model.connections"))
            {
                if (v.second.get("type", "").compare("i2c") == 0)
                {
                    bus_name = v.second.get("bus-name", bus_name);
                    bus_address = v.second.get("bus-address", bus_address);
                    break;
                }
            }
        }

        /* Add time tick callback to update thermal model */
        _time_bus->add_time_tick_callback(std::bind(&HeaterHardwareModel::update_thermal_model, this));

        _i2c_slave_connection = new I2CSlaveConnection(this, bus_address, connection_string, bus_name);
        sim_logger->info("HeaterHardwareModel::HeaterHardwareModel:  Now on I2C bus name %s as address 0x%02x.", bus_name.c_str(), bus_address);

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
        sim_logger->info("HeaterHardwareModel::HeaterHardwareModel:  Now on command bus named %s.", _command_bus_name.c_str());

        /* Initialize heater registers */
        _status_register = 0x00;  // Power OFF
        _temperature_register = static_cast<int16_t>(_heater_temperature * 100); // Store as centi-degrees

        sim_logger->info("HeaterHardwareModel::HeaterHardwareModel:  Initial temperature = %.2f°C, power state = %s",
            _heater_temperature, _power_state ? "ON" : "OFF");

        /* Construction complete */
        sim_logger->info("HeaterHardwareModel::HeaterHardwareModel:  Construction complete.");
    }


    HeaterHardwareModel::~HeaterHardwareModel(void)
    {
        /* Close the protocol bus */
        delete _i2c_slave_connection;
        _i2c_slave_connection = nullptr;

        /* Clean up the data provider */
        delete _heater_dp;
        _heater_dp = nullptr;

        /* The bus will clean up the time and sim nodes */
    }


    /* Automagically set up by the base class to be called */
    void HeaterHardwareModel::command_callback(NosEngine::Common::Message msg)
    {
        /* Get the data out of the message */
        NosEngine::Common::DataBufferOverlay dbf(const_cast<NosEngine::Utility::Buffer&>(msg.buffer));
        sim_logger->info("HeaterHardwareModel::command_callback:  Received command: %s.", dbf.data);

        /* Do something with the data */
        std::string command = dbf.data;
        std::string response = "HeaterHardwareModel::command_callback:  INVALID COMMAND! (Try HELP)";
        boost::to_upper(command);

        if (command.compare("HELP") == 0)
        {
            response = "HeaterHardwareModel::command_callback: Valid commands are HELP, ENABLE, DISABLE, TEMPERATURE=X, POWER_ON, POWER_OFF, or STOP";
        }
        else if (command.compare("ENABLE") == 0)
        {
            _enabled = HEATER_SIM_SUCCESS;
            response = "HeaterHardwareModel::command_callback:  Enabled (sent from EPS switch ON)";
            _power_state = true;
            _power_register = 0xAA;
            _status_register = 0x01; // Set power bit
        }
        else if (command.compare("DISABLE") == 0)
        {
            _enabled = HEATER_SIM_ERROR;
            response = "HeaterHardwareModel::command_callback:  Disabled (sent from EPS switch OFF)";
            _power_state = false;
            _power_register = 0x00;
            _status_register = 0x00; // Clear power bit
        }
        else if (command.compare("POWER_ON") == 0)
        {
            _power_state = true;
            _power_register = 0xAA;
            _status_register = 0x01;
            response = "HeaterHardwareModel::command_callback:  Power ON";
        }
        else if (command.compare("POWER_OFF") == 0)
        {
            _power_state = false;
            _power_register = 0x00;
            _status_register = 0x00;
            response = "HeaterHardwareModel::command_callback:  Power OFF";
        }
        else if (command.substr(0,12).compare("TEMPERATURE=") == 0)
        {
            try
            {
                _heater_temperature = std::stod(command.substr(12));
                _temperature_register = static_cast<int16_t>(_heater_temperature * 100);
                response = "HeaterHardwareModel::command_callback:  Temperature set to " + std::to_string(_heater_temperature) + "°C";
            }
            catch (...)
            {
                response = "HeaterHardwareModel::command_callback:  Temperature invalid";
            }
        }
        else if (command.compare("STOP") == 0)
        {
            _keep_running = false;
            response = "HeaterHardwareModel::command_callback:  Stopping";
        }

        /* Send a reply */
        sim_logger->info("HeaterHardwareModel::command_callback:  Sending reply: %s.", response.c_str());
        _command_node->send_reply_message_async(msg, response.size(), response.c_str());
    }

    /* Update thermal model based on heater power state */
    void HeaterHardwareModel::update_thermal_model(void)
    {
        // Get ambient temperature from data provider
        boost::shared_ptr<HeaterDataPoint> data_point = boost::dynamic_pointer_cast<HeaterDataPoint>(_heater_dp->get_data_point());
        if (data_point && data_point->is_data_valid()) {
            _ambient_temperature = data_point->get_ambient_temperature();
        }

        // Calculate temperature change based on power state
        double temp_change = 0.0;
        double time_step = _sim_microseconds_per_tick / 1000000.0; // Convert to seconds

        if (_power_state) {
            // Heater ON: temperature rises toward max temperature
            double temp_diff = _max_temperature - _heater_temperature;
            if (temp_diff > 0) {
                temp_change = _heating_rate * time_step;
                if (temp_change > temp_diff) {
                    temp_change = temp_diff; // Don't overshoot max
                }
            }
        } else {
            // Heater OFF: temperature cools toward ambient
            double temp_diff = _heater_temperature - _ambient_temperature;
            if (temp_diff > 0) {
                temp_change = -_cooling_rate * time_step;
                if (-temp_change > temp_diff) {
                    temp_change = -temp_diff; // Don't overshoot ambient
                }
            }
        }

        // Update heater temperature
        _heater_temperature += temp_change;
        _temperature_register = static_cast<int16_t>(_heater_temperature * 100);

        // Debug logging (only log occasionally to avoid spam)
        static int log_counter = 0;
        if (++log_counter >= 1000) { // Log every 1000 ticks (~10 seconds at 10ms/tick)
            sim_logger->debug("HeaterHardwareModel::update_thermal_model: Power=%s, Temp=%.2f°C, Ambient=%.2f°C",
                _power_state ? "ON" : "OFF", _heater_temperature, _ambient_temperature);
            log_counter = 0;
        }

        // Send temperature update to TMP100 simulator via command bus
        // This allows the TMP100 to read the environmental temperature including heater influence
        if (_command_node) {
            std::string tmp100_msg = "TEMPERATURE=" + std::to_string(_heater_temperature);
            _command_node->send_non_confirmed_message_async("tmp100-sim-command-node",
                tmp100_msg.length(), tmp100_msg.c_str());
        }
    }

    /* Protocol callback */
    std::uint8_t HeaterHardwareModel::determine_i2c_response_for_request(const std::vector<uint8_t>& in_data, std::vector<uint8_t>& out_data)
    {
        std::uint8_t valid = HEATER_SIM_SUCCESS;

        /* Retrieve data and log in human readable format */
        sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  REQUEST %s",
            SimIHardwareModel::uint8_vector_to_hex_string(in_data).c_str());

        /* Check simulator is enabled */
        if (_enabled != HEATER_SIM_SUCCESS)
        {
            sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  Heater sim disabled!");
            valid = HEATER_SIM_ERROR;
        }
        else
        {
            if (in_data.size() == 1)
            {
                // Write to pointer register
                _pointer_register = in_data[0];
                sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  Set pointer register to 0x%02x", _pointer_register);
            }
            else if (in_data.size() == 0)
            {
                // Read from current pointer register
                switch (_pointer_register)
                {
                    case HEATER_REG_STATUS:
                        out_data.resize(1);
                        out_data[0] = _status_register;
                        sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  Read status: 0x%02x", _status_register);
                        break;

                    case HEATER_REG_POWER:
                        out_data.resize(1);
                        out_data[0] = _power_register;
                        sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  Read power: 0x%02x", _power_register);
                        break;

                    case HEATER_REG_TEMP:
                        out_data.resize(2);
                        out_data[0] = (_temperature_register >> 8) & 0xFF;
                        out_data[1] = _temperature_register & 0xFF;
                        sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  Read temperature: 0x%04x (%.2f°C)",
                            _temperature_register, _heater_temperature);
                        break;

                    default:
                        sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  Invalid register 0x%02x", _pointer_register);
                        valid = HEATER_SIM_ERROR;
                        break;
                }
            }
            else if (in_data.size() == 2)
            {
                // Write to register
                uint8_t reg = in_data[0];
                uint8_t value = in_data[1];

                switch (reg)
                {
                    case HEATER_REG_POWER:
                        _power_register = value;
                        _power_state = (value == 0xAA);
                        _status_register = _power_state ? 0x01 : 0x00;
                        sim_logger->info("HeaterHardwareModel::determine_i2c_response_for_request:  Power set to %s (0x%02x)",
                            _power_state ? "ON" : "OFF", value);
                        break;

                    default:
                        sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  Write to register 0x%02x not allowed", reg);
                        valid = HEATER_SIM_ERROR;
                        break;
                }
            }
            else
            {
                sim_logger->debug("HeaterHardwareModel::determine_i2c_response_for_request:  Invalid command size %ld", in_data.size());
                valid = HEATER_SIM_ERROR;
            }
        }

        return valid;
    }

    I2CSlaveConnection::I2CSlaveConnection(HeaterHardwareModel* hm,
        int bus_address, std::string connection_string, std::string bus_name)
        : NosEngine::I2C::I2CSlave(bus_address, connection_string, bus_name)
    {
        _hardware_model = hm;
    }

    size_t I2CSlaveConnection::i2c_read(uint8_t *rbuf, size_t rlen)
    {
        size_t num_read;
        if(_i2c_read_valid == HEATER_SIM_SUCCESS)
        {
            for(num_read = 0; num_read < rlen && num_read < _i2c_out_data.size(); num_read++)
            {
                rbuf[num_read] = _i2c_out_data[num_read];
            }
            sim_logger->debug("i2c_read[%ld]: %s", num_read, SimIHardwareModel::uint8_vector_to_hex_string(_i2c_out_data).c_str());
        }
        else
        {
            for(num_read = 0; num_read < rlen; num_read++)
            {
                rbuf[num_read] = 0x00;
            }
            sim_logger->debug("i2c_read[%ld]: Invalid (0x00)", num_read);
        }

        return num_read;
    }

    size_t I2CSlaveConnection::i2c_write(const uint8_t *wbuf, size_t wlen)
    {
        std::vector<uint8_t> in_data(wbuf, wbuf + wlen);
        sim_logger->debug("i2c_write: %s",
            SimIHardwareModel::uint8_vector_to_hex_string(in_data).c_str()); // log data
        _i2c_read_valid = _hardware_model->determine_i2c_response_for_request(in_data, _i2c_out_data);
        return wlen;
    }
}
