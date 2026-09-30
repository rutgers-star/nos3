#include <payload_if_hardware_model.hpp>

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

namespace Nos3
{
    REGISTER_HARDWARE_MODEL(Payload_ifHardwareModel,"PAYLOAD_IF");

    extern ItcLogger::Logger *sim_logger;

    namespace
    {
        /* Decimal, or hexadecimal with a 0X prefix (commands arrive upper-cased) */
        unsigned long parse_number(const std::string& text)
        {
            size_t used = 0;
            const bool hex = text.compare(0, 2, "0X") == 0;
            unsigned long value = std::stoul(hex ? text.substr(2) : text, &used, hex ? 16 : 10);
            if (used != (hex ? text.size() - 2 : text.size()))
            {
                throw std::invalid_argument(text);
            }
            return value;
        }
    }

    Payload_ifHardwareModel::Payload_ifHardwareModel(const boost::property_tree::ptree& config) : SimIHardwareModel(config),
    _model(config.get("simulator.hardware-model.initial-sequence-count", 0)), _enabled(true),
    _seed(config.get("simulator.hardware-model.initial-sequence-count", 0)), _tx_credit(0.0)
    {
        /* Get the NOS engine connection string */
        std::string connection_string = config.get("common.nos-connection-string", "tcp://127.0.0.1:12001");
        sim_logger->info("Payload_ifHardwareModel::Payload_ifHardwareModel:  NOS Engine connection string: %s.", connection_string.c_str());

        /* Get on the UART to the BusOBC and the time bus */
        /* Note: Initialized defaults in case value not found in config file */
        std::string bus_name = "usart_17";
        int node_port = 17;
        int baud = 115200;
        std::string time_bus_name = "command";
        if (config.get_child_optional("simulator.hardware-model.connections"))
        {
            /* Loop through the connections for hardware model */
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("simulator.hardware-model.connections"))
            {
                /* v.second is the child tree (v.first is the name of the child) */
                if (v.second.get("type", "").compare("usart") == 0)
                {
                    bus_name = v.second.get("bus-name", bus_name);
                    node_port = v.second.get("node-port", node_port);
                    baud = v.second.get("baud", baud);
                }
                else if (v.second.get("type", "").compare("time") == 0)
                {
                    time_bus_name = v.second.get("bus-name", time_bus_name);
                }
            }
        }

        /*
        ** 8N1 framing puts 10 bits on the wire per byte. Output is paced to that
        ** line rate so a large or doubled response reaches the BusOBC in several
        ** UART writes, as it would from real hardware.
        */
        double seconds_per_tick = config.get("common.sim-microseconds-per-tick", 1000000) / 1000000.0;
        _tx_bytes_per_tick = std::max(1.0, (baud / 10.0) * seconds_per_tick);

        _model.set_log([](const std::string& message)
        {
            sim_logger->info("Payload_ifHardwareModel:  %s", message.c_str());
        });

        _uart_connection.reset(new NosEngine::Uart::Uart(_hub, config.get("simulator.name", "payload_if_sim"), connection_string, bus_name));
        _uart_connection->open(node_port);
        _uart_connection->set_read_callback(std::bind(&Payload_ifHardwareModel::uart_read_callback, this, std::placeholders::_1, std::placeholders::_2));
        sim_logger->info("Payload_ifHardwareModel::Payload_ifHardwareModel:  Now on UART bus name %s, port %d, %d baud (%.1f bytes per tick).",
            bus_name.c_str(), node_port, baud, _tx_bytes_per_tick);

        _time_bus.reset(new NosEngine::Client::Bus(_hub, connection_string, time_bus_name));
        _time_bus->add_time_tick_callback(std::bind(&Payload_ifHardwareModel::time_tick_callback, this, std::placeholders::_1));
        sim_logger->info("Payload_ifHardwareModel::Payload_ifHardwareModel:  Now on time bus named %s.", time_bus_name.c_str());

        /* Construction complete */
        sim_logger->info("Payload_ifHardwareModel::Payload_ifHardwareModel:  Construction complete, sequence count seed %u.", _seed);
    }


    Payload_ifHardwareModel::~Payload_ifHardwareModel(void)
    {
        /* Stop the callbacks before the model they use is destroyed */
        _time_bus.reset();
        _uart_connection->close();
    }


    /* Bytes written by the BusOBC; any chunk size, including partial and combined frames */
    void Payload_ifHardwareModel::uart_read_callback(const uint8_t *buf, size_t len)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        sim_logger->debug("Payload_ifHardwareModel::uart_read_callback:  RX %s",
            SimIHardwareModel::uint8_vector_to_hex_string(std::vector<uint8_t>(buf, buf + len)).c_str());

        if (_enabled)
        {
            _model.receive(buf, len);
        }
    }


    /* Transmit queued bytes at no more than the UART line rate */
    void Payload_ifHardwareModel::time_tick_callback(NosEngine::Common::SimTime time)
    {
        (void)time;
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_enabled || _model.pending_tx_bytes() == 0)
        {
            /* An idle line does not bank credit for a later burst */
            _tx_credit = 0.0;
            return;
        }

        _tx_credit += _tx_bytes_per_tick;
        std::vector<uint8_t> out = _model.drain(static_cast<size_t>(_tx_credit));
        if (!out.empty())
        {
            _tx_credit -= out.size();
            size_t written = _uart_connection->write(out.data(), out.size());
            sim_logger->info("Payload_ifHardwareModel::time_tick_callback:  TX %s",
                SimIHardwareModel::uint8_vector_to_hex_string(out).c_str());
            if (written != out.size())
            {
                sim_logger->error("Payload_ifHardwareModel::time_tick_callback:  UART accepted %zu of %zu bytes; the rest are lost",
                    written, out.size());
            }
        }
    }


    /* Automagically set up by the base class to be called */
    void Payload_ifHardwareModel::command_callback(NosEngine::Common::Message msg)
    {
        /* Get the data out of the message */
        NosEngine::Common::DataBufferOverlay dbf(const_cast<NosEngine::Utility::Buffer&>(msg.buffer));
        sim_logger->info("Payload_ifHardwareModel::command_callback:  Received command: %s.", dbf.data);

        std::string command = dbf.data;
        boost::to_upper(command);
        std::string response = "Payload_ifHardwareModel::command_callback:  " + process_command(command);

        /* Send a reply */
        sim_logger->info("%s", response.c_str());
        _command_node->send_reply_message_async(msg, response.size(), response.c_str());
    }


    std::string Payload_ifHardwareModel::process_command(const std::string& command)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (command.compare("HELP") == 0)
        {
            return "Valid commands are HELP, ENABLE, DISABLE, STOP, COUNTERS, RESET[=SEED], SEND_STATUS, "
                   "CORRUPT_CRC, SPLIT[=BYTES], DOUBLE, BAD_LENGTH, BAD_APID";
        }
        if (command.compare("ENABLE") == 0)
        {
            _enabled = true;
            return "Enabled";
        }
        if (command.compare("DISABLE") == 0)
        {
            /* Ignore UART input and hold queued output; RESET clears state */
            _enabled = false;
            return "Disabled";
        }
        if (command.compare("STOP") == 0)
        {
            _keep_running = false;
            return "Stopping";
        }
        if (command.compare("COUNTERS") == 0)
        {
            return counters_string();
        }
        if (command.compare(0, 5, "RESET") == 0)
        {
            if (command.size() > 5)
            {
                if (command[5] != '=')
                {
                    return "INVALID COMMAND! (Try HELP)";
                }
                try
                {
                    unsigned long seed = parse_number(command.substr(6));
                    if (seed > 0x3FFF)
                    {
                        return "RESET seed must be a 14-bit sequence count (0 to 0x3FFF)";
                    }
                    _seed = static_cast<std::uint16_t>(seed);
                }
                catch (...)
                {
                    return "RESET seed invalid";
                }
            }
            _model.reset(_seed);
            _tx_credit = 0.0;
            return "Reset, sequence count seed " + std::to_string(_seed);
        }
        if (command.compare("SEND_STATUS") == 0)
        {
            _model.request_status();
            return "Status telemetry queued";
        }
        if (command.compare("CORRUPT_CRC") == 0)
        {
            _model.corrupt_next_crc();
            return "Next outgoing frame will have a corrupt CRC";
        }
        if (command.compare(0, 5, "SPLIT") == 0)
        {
            size_t first_part_len = 0;
            if (command.size() > 5)
            {
                if (command[5] != '=')
                {
                    return "INVALID COMMAND! (Try HELP)";
                }
                try
                {
                    first_part_len = parse_number(command.substr(6));
                }
                catch (...)
                {
                    return "SPLIT byte count invalid";
                }
            }
            _model.split_next_frame(first_part_len);
            return "Next outgoing frame will be split across UART writes";
        }
        if (command.compare("DOUBLE") == 0)
        {
            _model.double_next_telemetry();
            return "Next telemetry will be sent as two back-to-back frames";
        }
        if (command.compare("BAD_LENGTH") == 0)
        {
            _model.bad_length_next_packet();
            return "Next outgoing packet will have an invalid CCSDS length";
        }
        if (command.compare("BAD_APID") == 0)
        {
            _model.bad_apid_next_packet();
            return "Next outgoing packet will use disallowed APID 0x010";
        }
        return "INVALID COMMAND! (Try HELP)";
    }


    std::string Payload_ifHardwareModel::counters_string(void) const
    {
        const PayObcLinkCounters& c = _model.counters();
        std::ostringstream ss;
        ss << "enabled=" << (_enabled ? 1 : 0)
           << " frames_received=" << c.frames_received
           << " frames_bad_crc=" << c.frames_bad_crc
           << " frames_resync=" << c.frames_resync
           << " packets_malformed=" << c.packets_malformed
           << " packets_disallowed=" << c.packets_disallowed
           << " packets_unhandled=" << c.packets_unhandled
           << " commands_accepted=" << c.commands_accepted
           << " frames_queued=" << c.frames_queued
           << " next_sequence_count=" << _model.next_sequence_count()
           << " pending_tx_bytes=" << _model.pending_tx_bytes();
        return ss.str();
    }
}
