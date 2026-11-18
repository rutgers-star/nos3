#ifndef NOS3_PAYOBC_LINK_MODEL_HPP
#define NOS3_PAYOBC_LINK_MODEL_HPP

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <vector>

#include <payload_link/frame.h>

/*
** Deterministic PayOBC link model.
**
** Consumes the BusOBC-to-PayOBC UART byte stream, decodes payload-link frames
** with the shared library, validates the CCSDS packet inside each frame, and
** answers payload commands (APID 0x010) with ACK/NACK (APID 0x012) and
** sends a heartbeat (APID 0x011) each simulated second. Bytes are queued and
** leave through drain(), which models a UART that accepts a limited number of
** bytes per write.
**
** This class has no NOS Engine dependency so it can be unit tested directly.
** It is not thread safe; the hardware model serializes access to it.
*/
namespace Nos3
{
    /* Counters exposed to tests and logged by the hardware model. */
    struct PayObcLinkCounters
    {
        std::uint32_t frames_received;   /* Frames that passed length and CRC checks */
        std::uint32_t frames_bad_crc;    /* Frames dropped for a CRC mismatch */
        std::uint32_t frames_resync;     /* Frames dropped for an invalid payload-link length */
        std::uint32_t packets_malformed; /* CCSDS header or length checks failed */
        std::uint32_t packets_disallowed;/* APID not allowed BusOBC -> PayOBC */
        std::uint32_t packets_unhandled; /* Allowed APID with no simulated behavior */
        std::uint32_t commands_accepted; /* Valid APID 0x010 commands processed */
        std::uint32_t frames_queued;     /* Frames placed on the transmit queue */
    };

    class PayObcLinkModel
    {
    public:
        static const std::size_t CCSDS_PRIMARY_HEADER_LEN = 6;

        typedef std::function<void(const std::string &)> LogFn;

        explicit PayObcLinkModel(std::uint16_t seed = 0);

        /* Optional diagnostic sink for dropped frames and packets. */
        void set_log(LogFn log);

        /* Feed an arbitrary chunk of received UART bytes. Decoder state persists across calls. */
        void receive(const std::uint8_t *data, std::size_t len);

        /*
        ** Remove up to max_bytes from the transmit queue for one UART write.
        ** A frame split by split_next_frame() is never returned in the same
        ** write as the bytes before the split point.
        */
        std::vector<std::uint8_t> drain(std::size_t max_bytes);

        /* Bytes waiting in the transmit queue. */
        std::size_t pending_tx_bytes(void) const;

        /* Advance the mission clock and heartbeat cadence by one simulation tick. */
        void advance_time(std::uint64_t elapsed_us);

        /* Queue a heartbeat immediately through simulator control. */
        void request_heartbeat(void);

        /* Restore power-on state: counters, decoder, transmit queue, faults. */
        void reset(std::uint16_t seed);

        /* One-shot faults apply to the next command reply or requested heartbeat. */
        void corrupt_next_crc(void);
        void split_next_frame(std::size_t first_part_len); /* 0 splits the frame in half */
        void double_next_telemetry(void);
        void bad_length_next_packet(void);
        void bad_apid_next_packet(void);

        const PayObcLinkCounters &counters(void) const { return _counters; }
        std::uint16_t next_sequence_count(void) const { return _tx_sequence_count; }

    private:
        struct TxSegment
        {
            std::vector<std::uint8_t> bytes;
            bool starts_new_write; /* Must not share a UART write with earlier bytes */
        };

        void handle_frame(const std::uint8_t *body, std::size_t len);
        void handle_command(const std::uint8_t *body, std::size_t len);
        void emit_heartbeat(bool apply_faults);
        void emit_telemetry(std::uint16_t apid, const std::vector<std::uint8_t> &data, bool apply_faults);
        std::vector<std::uint8_t> encode_telemetry(std::uint16_t apid, const std::vector<std::uint8_t> &data,
                                                   bool apply_faults);
        void queue_frames(const std::vector<std::uint8_t> &frames, std::size_t split_at);
        void log(const std::string &message) const;

        plframe_decode_ctx_t _decoder;
        PayObcLinkCounters _counters;
        std::uint16_t _tx_sequence_count;
        std::uint64_t _met_us;
        std::uint64_t _heartbeat_elapsed_us;
        std::uint8_t _op_state;
        std::uint8_t _experiment_id;
        std::deque<TxSegment> _tx_queue;
        LogFn _log;

        bool _fault_corrupt_crc;
        bool _fault_split;
        std::size_t _fault_split_len;
        bool _fault_double;
        bool _fault_bad_length;
        bool _fault_bad_apid;
    };
}

#endif
