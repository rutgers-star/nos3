#include <payobc_link_model.hpp>

#include <algorithm>
#include <cstdio>

#include <star/payload_apids.h>

namespace Nos3
{
    const std::size_t PayObcLinkModel::STATUS_USER_DATA_LEN;
    const std::size_t PayObcLinkModel::CCSDS_PRIMARY_HEADER_LEN;
    const std::size_t PayObcLinkModel::STATUS_PACKET_LEN;
    const std::uint8_t PayObcLinkModel::REPORT_COMMAND;
    const std::uint8_t PayObcLinkModel::REPORT_REQUESTED;

    namespace
    {
        /* CCSDS primary header fields, first 16 bits */
        const std::uint16_t CCSDS_VERSION_MASK = 0xE000;
        const std::uint16_t CCSDS_TYPE_CMD = 0x1000;
        const std::uint16_t CCSDS_SEC_HDR_FLAG = 0x0800;
        const std::uint16_t CCSDS_APID_MASK = 0x07FF;

        /* Second 16 bits: sequence flags and sequence count */
        const std::uint16_t CCSDS_SEQ_FLAGS_MASK = 0xC000;
        const std::uint16_t CCSDS_SEQ_FLAGS_UNSEGMENTED = 0xC000;
        const std::uint16_t CCSDS_SEQ_COUNT_MASK = 0x3FFF;

        std::uint16_t read_be16(const std::uint8_t *p)
        {
            return static_cast<std::uint16_t>((p[0] << 8) | p[1]);
        }

        void append_be16(std::vector<std::uint8_t> &out, std::uint16_t value)
        {
            out.push_back(static_cast<std::uint8_t>(value >> 8));
            out.push_back(static_cast<std::uint8_t>(value & 0xFF));
        }

        std::uint16_t low16(std::uint32_t value)
        {
            return static_cast<std::uint16_t>(value & 0xFFFF);
        }

        std::string hex_bytes(const std::uint8_t *data, std::size_t len)
        {
            std::string out;
            char buf[4];
            for (std::size_t i = 0; i < len; i++)
            {
                std::snprintf(buf, sizeof(buf), "%02X", data[i]);
                out += buf;
            }
            return out;
        }

        std::string hex16(std::uint16_t value)
        {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "0x%03X", value);
            return buf;
        }
    }

    PayObcLinkModel::PayObcLinkModel(std::uint16_t seed)
    {
        reset(seed);
    }

    void PayObcLinkModel::set_log(LogFn log)
    {
        _log = log;
    }

    void PayObcLinkModel::reset(std::uint16_t seed)
    {
        plframe_decode_init(&_decoder);
        _counters = PayObcLinkCounters();
        _tx_sequence_count = static_cast<std::uint16_t>(seed & CCSDS_SEQ_COUNT_MASK);
        _tx_queue.clear();
        _fault_corrupt_crc = false;
        _fault_split = false;
        _fault_split_len = 0;
        _fault_double = false;
        _fault_bad_length = false;
        _fault_bad_apid = false;
    }

    void PayObcLinkModel::receive(const std::uint8_t *data, std::size_t len)
    {
        for (std::size_t i = 0; i < len; i++)
        {
            switch (plframe_decode_feed(&_decoder, data[i]))
            {
                case PL_DECODE_OK:
                    _counters.frames_received++;
                    handle_frame(_decoder.body, _decoder.body_len);
                    break;

                case PL_DECODE_BAD_CRC:
                    _counters.frames_bad_crc++;
                    log("dropped frame: bad CRC");
                    break;

                case PL_DECODE_RESYNC:
                    _counters.frames_resync++;
                    log("dropped frame: invalid payload-link length");
                    break;

                case PL_DECODE_NEED_MORE:
                default:
                    break;
            }
        }
    }

    void PayObcLinkModel::handle_frame(const std::uint8_t *body, std::size_t len)
    {
        /* payload-link guarantees PL_MIN_BODY_LEN, which covers the primary header */
        const std::uint16_t id = read_be16(&body[0]);
        const std::uint16_t sequence = read_be16(&body[2]);
        const std::size_t ccsds_len = static_cast<std::size_t>(read_be16(&body[4])) + 1 + CCSDS_PRIMARY_HEADER_LEN;
        const std::uint16_t apid = id & CCSDS_APID_MASK;

        if ((id & CCSDS_VERSION_MASK) != 0)
        {
            _counters.packets_malformed++;
            log("dropped packet: CCSDS version is not 0");
        }
        else if (ccsds_len != len)
        {
            _counters.packets_malformed++;
            log("dropped packet: CCSDS length implies " + std::to_string(ccsds_len) + " bytes, frame body has " +
                std::to_string(len));
        }
        else if ((sequence & CCSDS_SEQ_FLAGS_MASK) != CCSDS_SEQ_FLAGS_UNSEGMENTED)
        {
            _counters.packets_malformed++;
            log("dropped packet: sequence flags are not unsegmented (11)");
        }
        else if (!star_payload_apid_allowed_bus_to_pay(apid))
        {
            _counters.packets_disallowed++;
            log("dropped packet: APID " + hex16(apid) + " is not allowed BusOBC -> PayOBC");
        }
        else if (apid == STAR_APID_PAYLOAD_COMMAND)
        {
            /* ICD RevB D6: payload commands are telecommands without a secondary header */
            if ((id & (CCSDS_TYPE_CMD | CCSDS_SEC_HDR_FLAG)) != CCSDS_TYPE_CMD)
            {
                _counters.packets_malformed++;
                log("dropped packet: APID 0x010 must be a telecommand without a secondary header");
            }
            else
            {
                handle_command(body, len);
            }
        }
        else
        {
            _counters.packets_unhandled++;
            log("ignored packet: APID " + hex16(apid) + " has no simulated behavior");
        }
    }

    void PayObcLinkModel::handle_command(const std::uint8_t *body, std::size_t len)
    {
        /*
        ** ICD section 5.2 layout: opcode (1 byte) then a uint16 command counter.
        ** Shorter commands are still accepted; missing fields report as zero.
        */
        const std::uint8_t *user_data = &body[CCSDS_PRIMARY_HEADER_LEN];
        const std::size_t user_len = len - CCSDS_PRIMARY_HEADER_LEN;
        const std::uint8_t opcode = user_len >= 1 ? user_data[0] : 0;
        const std::uint16_t command_counter = user_len >= 3 ? read_be16(&user_data[1]) : 0;

        _counters.commands_accepted++;
        log("accepted command packet " + hex_bytes(body, len));
        emit_status(REPORT_COMMAND, opcode, command_counter);
    }

    void PayObcLinkModel::request_status(void)
    {
        emit_status(REPORT_REQUESTED, 0, 0);
    }

    void PayObcLinkModel::emit_status(std::uint8_t report_type, std::uint8_t opcode, std::uint16_t command_counter)
    {
        std::vector<std::uint8_t> frames = encode_frame(build_status_packet(report_type, opcode, command_counter));
        const std::size_t first_frame_len = frames.size();
        std::size_t split_at = 0;

        if (_fault_double)
        {
            std::vector<std::uint8_t> second = encode_frame(build_status_packet(report_type, opcode, command_counter));
            frames.insert(frames.end(), second.begin(), second.end());
            _fault_double = false;
        }

        if (_fault_split)
        {
            /* The split applies to the first frame; a doubled second frame follows it directly */
            split_at = _fault_split_len == 0 ? first_frame_len / 2 : _fault_split_len;
            split_at = std::min(split_at, first_frame_len - 1);
            _fault_split = false;
        }

        queue_frames(frames, split_at);
    }

    std::vector<std::uint8_t> PayObcLinkModel::build_status_packet(std::uint8_t report_type, std::uint8_t opcode,
                                                                   std::uint16_t command_counter)
    {
        std::vector<std::uint8_t> packet;
        std::uint16_t apid = STAR_APID_PAYLOAD_TELEMETRY;
        std::uint16_t length_field = STATUS_USER_DATA_LEN - 1;

        if (_fault_bad_apid)
        {
            /* A real APID in the wrong direction: BusOBC -> PayOBC only */
            apid = STAR_APID_PAYLOAD_COMMAND;
            _fault_bad_apid = false;
        }
        if (_fault_bad_length)
        {
            length_field++;
            _fault_bad_length = false;
        }

        packet.reserve(STATUS_PACKET_LEN);

        /* ICD RevB D6: telemetry type, no secondary header, unsegmented */
        append_be16(packet, apid);
        append_be16(packet, static_cast<std::uint16_t>(CCSDS_SEQ_FLAGS_UNSEGMENTED | _tx_sequence_count));
        append_be16(packet, length_field);

        packet.push_back(report_type);
        packet.push_back(opcode);
        append_be16(packet, command_counter);
        append_be16(packet, low16(_counters.commands_accepted));
        append_be16(packet, low16(_counters.frames_bad_crc));
        append_be16(packet, low16(_counters.frames_resync));
        append_be16(packet, low16(_counters.packets_malformed));
        append_be16(packet, low16(_counters.packets_disallowed));

        _tx_sequence_count = static_cast<std::uint16_t>((_tx_sequence_count + 1) & CCSDS_SEQ_COUNT_MASK);
        return packet;
    }

    std::vector<std::uint8_t> PayObcLinkModel::encode_frame(const std::vector<std::uint8_t> &packet)
    {
        std::vector<std::uint8_t> frame(PL_MAX_FRAME_LEN);
        const std::size_t frame_len = plframe_encode(packet.data(), packet.size(), frame.data());

        frame.resize(frame_len);
        if (frame_len > 0 && _fault_corrupt_crc)
        {
            frame[frame_len - 1] ^= 0xFF;
            _fault_corrupt_crc = false;
        }
        _counters.frames_queued++;
        return frame;
    }

    void PayObcLinkModel::queue_frames(const std::vector<std::uint8_t> &frames, std::size_t split_at)
    {
        if (split_at == 0)
        {
            _tx_queue.push_back(TxSegment{frames, false});
        }
        else
        {
            _tx_queue.push_back(TxSegment{std::vector<std::uint8_t>(frames.begin(), frames.begin() + split_at), false});
            _tx_queue.push_back(TxSegment{std::vector<std::uint8_t>(frames.begin() + split_at, frames.end()), true});
        }
    }

    std::vector<std::uint8_t> PayObcLinkModel::drain(std::size_t max_bytes)
    {
        std::vector<std::uint8_t> out;

        while (!_tx_queue.empty() && out.size() < max_bytes)
        {
            TxSegment &segment = _tx_queue.front();
            if (segment.starts_new_write && !out.empty())
            {
                break;
            }

            const std::size_t take = std::min(max_bytes - out.size(), segment.bytes.size());
            out.insert(out.end(), segment.bytes.begin(), segment.bytes.begin() + take);
            if (take == segment.bytes.size())
            {
                _tx_queue.pop_front();
            }
            else
            {
                /* The remainder continues the same segment, so it may join the next write */
                segment.bytes.erase(segment.bytes.begin(), segment.bytes.begin() + take);
                segment.starts_new_write = false;
            }
        }
        return out;
    }

    std::size_t PayObcLinkModel::pending_tx_bytes(void) const
    {
        std::size_t total = 0;
        for (const TxSegment &segment : _tx_queue)
        {
            total += segment.bytes.size();
        }
        return total;
    }

    void PayObcLinkModel::corrupt_next_crc(void)
    {
        _fault_corrupt_crc = true;
    }

    void PayObcLinkModel::split_next_frame(std::size_t first_part_len)
    {
        _fault_split = true;
        _fault_split_len = first_part_len;
    }

    void PayObcLinkModel::double_next_telemetry(void)
    {
        _fault_double = true;
    }

    void PayObcLinkModel::bad_length_next_packet(void)
    {
        _fault_bad_length = true;
    }

    void PayObcLinkModel::bad_apid_next_packet(void)
    {
        _fault_bad_apid = true;
    }

    void PayObcLinkModel::log(const std::string &message) const
    {
        if (_log)
        {
            _log(message);
        }
    }
}
