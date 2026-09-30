/*
** Unit tests for the deterministic PayOBC link model.
**
** Expected frames are literal byte vectors computed independently with
** Python binascii.crc_hqx (CRC-16/CCITT-FALSE over LENGTH + BODY), not with
** the payload-link encoder under test. The command vector is the ICD RevB A1
** frame test vector.
*/
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include <payload_link/frame.h>
#include <star/payload_apids.h>

#include <payobc_link_model.hpp>

using Bytes = std::vector<std::uint8_t>;
using Nos3::PayObcLinkModel;

namespace
{
    /* ICD RevB A1: APID 0x010 telecommand, seq 0, opcode 0x20, counter 1, argument 1 */
    const Bytes CMD_FRAME = {0x1A, 0xCF, 0xFC, 0x1D, 0x00, 0x0D, 0x10, 0x10, 0xC0, 0x00, 0x00,
                             0x06, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x9B, 0xBA};

    /* Same command, sequence count 1, command counter 2 */
    const Bytes CMD2_FRAME = {0x1A, 0xCF, 0xFC, 0x1D, 0x00, 0x0D, 0x10, 0x10, 0xC0, 0x01, 0x00,
                              0x06, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x1A, 0x2D};

    /* Status response to CMD_FRAME after reset(0): seq 0, opcode 0x20, counter 1, accepted 1 */
    const Bytes RSP0_FRAME = {0x1A, 0xCF, 0xFC, 0x1D, 0x00, 0x14, 0x00, 0x11, 0xC0, 0x00, 0x00, 0x0D, 0x01, 0x20,
                              0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x68};

    /* Status response to CMD2_FRAME following RSP0: seq 1, counter 2, accepted 2 */
    const Bytes RSP1_FRAME = {0x1A, 0xCF, 0xFC, 0x1D, 0x00, 0x14, 0x00, 0x11, 0xC0, 0x01, 0x00, 0x0D, 0x01, 0x20,
                              0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB6, 0x34};

    /* Requested status after reset(0x1234): seq 0x1234, report type 2, all counters zero */
    const Bytes REQ_STATUS_SEED_FRAME = {0x1A, 0xCF, 0xFC, 0x1D, 0x00, 0x14, 0x00, 0x11, 0xD2, 0x34,
                                         0x00, 0x0D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFE};

    const std::size_t UNLIMITED = 1u << 20;

    void feed(PayObcLinkModel &model, const Bytes &bytes)
    {
        model.receive(bytes.data(), bytes.size());
    }

    Bytes concat(const Bytes &a, const Bytes &b)
    {
        Bytes out(a);
        out.insert(out.end(), b.begin(), b.end());
        return out;
    }

    Bytes frame_of(const Bytes &packet)
    {
        Bytes frame(PL_MAX_FRAME_LEN);
        frame.resize(plframe_encode(packet.data(), packet.size(), frame.data()));
        return frame;
    }

    /* Valid APID 0x010 command packet with the given user data length (>= 1) */
    Bytes command_packet(std::size_t user_len, std::uint8_t opcode = 0x20)
    {
        Bytes packet = {0x10, 0x10, 0xC0, 0x00, static_cast<std::uint8_t>((user_len - 1) >> 8),
                        static_cast<std::uint8_t>((user_len - 1) & 0xFF)};
        for (std::size_t i = 0; i < user_len; i++)
        {
            packet.push_back(static_cast<std::uint8_t>(i));
        }
        packet[6] = opcode;
        return packet;
    }

    /* Decode every frame in a byte stream with a fresh decoder */
    struct Decoded
    {
        std::vector<Bytes> bodies;
        int bad_crc = 0;
        int resync = 0;
    };

    Decoded decode_all(const Bytes &stream)
    {
        Decoded out;
        plframe_decode_ctx_t ctx;
        plframe_decode_init(&ctx);
        for (std::uint8_t b : stream)
        {
            switch (plframe_decode_feed(&ctx, b))
            {
                case PL_DECODE_OK:
                    out.bodies.push_back(Bytes(ctx.body, ctx.body + ctx.body_len));
                    break;
                case PL_DECODE_BAD_CRC:
                    out.bad_crc++;
                    break;
                case PL_DECODE_RESYNC:
                    out.resync++;
                    break;
                default:
                    break;
            }
        }
        return out;
    }
}

TEST(PayObcLinkModel, LiteralCommandProducesLiteralResponse)
{
    PayObcLinkModel model(0);
    feed(model, CMD_FRAME);

    EXPECT_EQ(model.drain(UNLIMITED), RSP0_FRAME);
    EXPECT_EQ(model.counters().frames_received, 1u);
    EXPECT_EQ(model.counters().commands_accepted, 1u);
    EXPECT_EQ(model.pending_tx_bytes(), 0u);
}

TEST(PayObcLinkModel, SequenceCountAndCounterAdvanceDeterministically)
{
    PayObcLinkModel model(0);
    feed(model, CMD_FRAME);
    EXPECT_EQ(model.drain(UNLIMITED), RSP0_FRAME);
    feed(model, CMD2_FRAME);
    EXPECT_EQ(model.drain(UNLIMITED), RSP1_FRAME);
    EXPECT_EQ(model.next_sequence_count(), 2u);

    /* Identical input to a second model gives identical output */
    PayObcLinkModel other(0);
    feed(other, concat(CMD_FRAME, CMD2_FRAME));
    EXPECT_EQ(other.drain(UNLIMITED), concat(RSP0_FRAME, RSP1_FRAME));
}

TEST(PayObcLinkModel, SequenceCountWrapsAt14Bits)
{
    PayObcLinkModel model(0x3FFF);
    model.request_status();
    model.request_status();
    Decoded d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 2u);
    EXPECT_EQ(d.bodies[0][2], 0xFF);
    EXPECT_EQ(d.bodies[0][3], 0xFF);
    EXPECT_EQ(d.bodies[1][2], 0xC0);
    EXPECT_EQ(d.bodies[1][3], 0x00);
}

TEST(PayObcLinkModel, MinimumAndMaximumBodySizesAccepted)
{
    PayObcLinkModel model(0);

    /* 7-byte body: primary header plus the opcode only; counter reports as 0 */
    feed(model, frame_of(command_packet(1)));
    Decoded d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 1u);
    EXPECT_EQ(d.bodies[0][7], 0x20); /* opcode */
    EXPECT_EQ(d.bodies[0][8], 0x00); /* counter high */
    EXPECT_EQ(d.bodies[0][9], 0x00); /* counter low */

    /* 518-byte body: the payload-link maximum */
    Bytes max_frame = frame_of(command_packet(PL_MAX_BODY_LEN - 6));
    ASSERT_EQ(max_frame.size(), static_cast<std::size_t>(PL_MAX_FRAME_LEN));
    feed(model, max_frame);
    d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 1u);
    EXPECT_EQ(model.counters().commands_accepted, 2u);
    EXPECT_EQ(model.counters().packets_malformed, 0u);
}

TEST(PayObcLinkModel, InvalidPayloadLinkLengthResyncsAndRecovers)
{
    PayObcLinkModel model(0);
    /* Length 6 is below the 7-byte minimum; 519 is above the 518-byte maximum */
    feed(model, {0x1A, 0xCF, 0xFC, 0x1D, 0x00, 0x06});
    feed(model, {0x1A, 0xCF, 0xFC, 0x1D, 0x02, 0x07});
    EXPECT_EQ(model.counters().frames_resync, 2u);
    EXPECT_EQ(model.pending_tx_bytes(), 0u);

    feed(model, CMD_FRAME);
    Decoded d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 1u);
    EXPECT_EQ(d.bodies[0][15], 0x02); /* frames_resync low byte reported in telemetry */
}

TEST(PayObcLinkModel, FrameSplitAcrossEveryBoundaryIsAccepted)
{
    for (std::size_t split = 1; split < CMD_FRAME.size(); split++)
    {
        PayObcLinkModel model(0);
        model.receive(CMD_FRAME.data(), split);
        EXPECT_EQ(model.pending_tx_bytes(), 0u) << "split " << split;
        model.receive(CMD_FRAME.data() + split, CMD_FRAME.size() - split);
        EXPECT_EQ(model.drain(UNLIMITED), RSP0_FRAME) << "split " << split;
    }
}

TEST(PayObcLinkModel, FrameFedOneByteAtATimeIsAccepted)
{
    PayObcLinkModel model(0);
    for (std::uint8_t b : CMD_FRAME)
    {
        model.receive(&b, 1);
    }
    EXPECT_EQ(model.drain(UNLIMITED), RSP0_FRAME);
}

TEST(PayObcLinkModel, TwoFramesInOneChunkAreBothProcessed)
{
    PayObcLinkModel model(0);
    feed(model, concat(CMD_FRAME, CMD2_FRAME));
    EXPECT_EQ(model.counters().commands_accepted, 2u);
    EXPECT_EQ(model.drain(UNLIMITED), concat(RSP0_FRAME, RSP1_FRAME));
}

TEST(PayObcLinkModel, CombinedChunkEndingMidFrameKeepsDecoderState)
{
    PayObcLinkModel model(0);
    Bytes stream = concat(CMD_FRAME, CMD2_FRAME);
    const std::size_t cut = CMD_FRAME.size() + 9;
    model.receive(stream.data(), cut);
    EXPECT_EQ(model.counters().commands_accepted, 1u);
    model.receive(stream.data() + cut, stream.size() - cut);
    EXPECT_EQ(model.counters().commands_accepted, 2u);
    EXPECT_EQ(model.drain(UNLIMITED), concat(RSP0_FRAME, RSP1_FRAME));
}

TEST(PayObcLinkModel, GarbageBeforeSyncIsSkipped)
{
    PayObcLinkModel model(0);
    /* Includes a partial sync word that must not capture the decoder */
    feed(model, concat({0x00, 0xFF, 0x1A, 0xCF, 0xFC, 0x55, 0x1A, 0x1A, 0xCF}, CMD_FRAME));
    EXPECT_EQ(model.drain(UNLIMITED), RSP0_FRAME);
    EXPECT_EQ(model.counters().frames_bad_crc, 0u);
    EXPECT_EQ(model.counters().frames_resync, 0u);
}

TEST(PayObcLinkModel, BadCrcIsCountedAndNextFrameSucceeds)
{
    PayObcLinkModel model(0);
    Bytes corrupt = CMD_FRAME;
    corrupt.back() ^= 0x01;
    feed(model, concat(corrupt, CMD_FRAME));

    EXPECT_EQ(model.counters().frames_bad_crc, 1u);
    EXPECT_EQ(model.counters().commands_accepted, 1u);
    Decoded d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 1u);
    EXPECT_EQ(d.bodies[0][12], 0x00); /* frames_bad_crc reported as 1 */
    EXPECT_EQ(d.bodies[0][13], 0x01);
}

TEST(PayObcLinkModel, CcsdsLengthMismatchIsDropped)
{
    PayObcLinkModel model(0);
    Bytes packet = command_packet(7);
    packet[5] = 0x07; /* claims 8 user data bytes, frame carries 7 */
    feed(model, frame_of(packet));

    EXPECT_EQ(model.counters().frames_received, 1u);
    EXPECT_EQ(model.counters().packets_malformed, 1u);
    EXPECT_EQ(model.counters().commands_accepted, 0u);
    EXPECT_EQ(model.pending_tx_bytes(), 0u);
}

TEST(PayObcLinkModel, MalformedPrimaryHeadersAreDropped)
{
    PayObcLinkModel model(0);

    Bytes version = command_packet(7);
    version[0] |= 0x20; /* version 1 */
    Bytes telemetry_type = command_packet(7);
    telemetry_type[0] = 0x00; /* APID 0x010 with the telemetry type bit */
    Bytes secondary_header = command_packet(7);
    secondary_header[0] = 0x18; /* APID 0x010 telecommand with a secondary header */
    Bytes segmented = command_packet(7);
    segmented[2] = 0x40; /* first segment, not unsegmented */

    for (const Bytes &packet : {version, telemetry_type, secondary_header, segmented})
    {
        feed(model, frame_of(packet));
    }
    EXPECT_EQ(model.counters().frames_received, 4u);
    EXPECT_EQ(model.counters().packets_malformed, 4u);
    EXPECT_EQ(model.counters().commands_accepted, 0u);
    EXPECT_EQ(model.pending_tx_bytes(), 0u);
}

TEST(PayObcLinkModel, ApidAllowlistFollowsPayloadApids)
{
    PayObcLinkModel model(0);

    /* PayOBC -> BusOBC only, protocol reserved, and unassigned APIDs are refused */
    const std::uint16_t disallowed[] = {STAR_APID_PAYLOAD_TELEMETRY, STAR_APID_IDLE, 0x100};
    for (std::uint16_t apid : disallowed)
    {
        ASSERT_FALSE(star_payload_apid_allowed_bus_to_pay(apid));
        Bytes packet = command_packet(7);
        packet[0] = static_cast<std::uint8_t>(0x10 | (apid >> 8));
        packet[1] = static_cast<std::uint8_t>(apid & 0xFF);
        feed(model, frame_of(packet));
    }
    EXPECT_EQ(model.counters().packets_disallowed, 3u);

    /* Allowed BusOBC -> PayOBC APID without simulated behavior: accepted, no response */
    ASSERT_TRUE(star_payload_apid_allowed_bus_to_pay(STAR_APID_ADCS_TELEMETRY_PASSTHROUGH));
    Bytes adcs = command_packet(7);
    adcs[0] = 0x00;
    adcs[1] = static_cast<std::uint8_t>(STAR_APID_ADCS_TELEMETRY_PASSTHROUGH);
    feed(model, frame_of(adcs));
    EXPECT_EQ(model.counters().packets_unhandled, 1u);
    EXPECT_EQ(model.pending_tx_bytes(), 0u);

    /* The payload command APID is allowed and answered */
    feed(model, CMD_FRAME);
    EXPECT_EQ(model.counters().commands_accepted, 1u);
    EXPECT_GT(model.pending_tx_bytes(), 0u);
}

TEST(PayObcLinkModel, PartialOutputIsDeliveredAcrossWrites)
{
    PayObcLinkModel model(0);
    feed(model, CMD_FRAME);

    Bytes collected;
    int writes = 0;
    while (model.pending_tx_bytes() > 0)
    {
        Bytes chunk = model.drain(5);
        ASSERT_LE(chunk.size(), 5u);
        ASSERT_FALSE(chunk.empty());
        collected.insert(collected.end(), chunk.begin(), chunk.end());
        writes++;
    }
    EXPECT_EQ(writes, 6); /* 28 bytes in writes of at most 5 */
    EXPECT_EQ(collected, RSP0_FRAME);
    EXPECT_TRUE(model.drain(5).empty());
}

TEST(PayObcLinkModel, FaultCorruptNextCrc)
{
    PayObcLinkModel model(0);
    model.corrupt_next_crc();
    feed(model, CMD_FRAME);
    Bytes first = model.drain(UNLIMITED);
    feed(model, CMD2_FRAME);
    Bytes second = model.drain(UNLIMITED);

    Bytes expected_first = RSP0_FRAME;
    expected_first.back() ^= 0xFF;
    EXPECT_EQ(first, expected_first);
    EXPECT_EQ(second, RSP1_FRAME); /* one-shot: the following frame is valid */

    Decoded d = decode_all(concat(first, second));
    EXPECT_EQ(d.bad_crc, 1);
    EXPECT_EQ(d.bodies.size(), 1u);
}

TEST(PayObcLinkModel, FaultSplitNextFrameAcrossWrites)
{
    PayObcLinkModel model(0);
    model.split_next_frame(10);
    feed(model, CMD_FRAME);

    Bytes part1 = model.drain(UNLIMITED);
    Bytes part2 = model.drain(UNLIMITED);
    EXPECT_EQ(part1, Bytes(RSP0_FRAME.begin(), RSP0_FRAME.begin() + 10));
    EXPECT_EQ(part2, Bytes(RSP0_FRAME.begin() + 10, RSP0_FRAME.end()));

    /* Default split point is half the frame; the fault is one-shot */
    model.split_next_frame(0);
    feed(model, CMD2_FRAME);
    EXPECT_EQ(model.drain(UNLIMITED).size(), RSP1_FRAME.size() / 2);
    EXPECT_EQ(model.drain(UNLIMITED).size(), RSP1_FRAME.size() - RSP1_FRAME.size() / 2);
    model.request_status();
    EXPECT_EQ(model.drain(UNLIMITED).size(), RSP0_FRAME.size());
}

TEST(PayObcLinkModel, FaultDoubleNextTelemetryInOneWrite)
{
    PayObcLinkModel model(0);
    model.double_next_telemetry();
    feed(model, CMD_FRAME);

    Bytes out = model.drain(UNLIMITED);
    Decoded d = decode_all(out);
    ASSERT_EQ(d.bodies.size(), 2u);
    EXPECT_EQ(Bytes(out.begin(), out.begin() + RSP0_FRAME.size()), RSP0_FRAME);
    EXPECT_EQ(d.bodies[1][3], 0x01); /* second packet carries the next sequence count */
    EXPECT_EQ(model.counters().frames_queued, 2u);

    /* Combined with a corrupt CRC: bad frame then valid frame in one write */
    model.double_next_telemetry();
    model.corrupt_next_crc();
    model.request_status();
    d = decode_all(model.drain(UNLIMITED));
    EXPECT_EQ(d.bad_crc, 1);
    EXPECT_EQ(d.bodies.size(), 1u);
}

TEST(PayObcLinkModel, FaultBadLengthNextPacket)
{
    PayObcLinkModel model(0);
    model.bad_length_next_packet();
    feed(model, CMD_FRAME);

    Decoded d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 1u); /* valid frame, inconsistent CCSDS packet */
    const Bytes &body = d.bodies[0];
    const std::size_t implied = static_cast<std::size_t>((body[4] << 8) | body[5]) + 7;
    EXPECT_EQ(implied, body.size() + 1);

    model.request_status();
    d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 1u);
    EXPECT_EQ(static_cast<std::size_t>((d.bodies[0][4] << 8) | d.bodies[0][5]) + 7, d.bodies[0].size());
}

TEST(PayObcLinkModel, FaultBadApidNextPacket)
{
    PayObcLinkModel model(0);
    model.bad_apid_next_packet();
    feed(model, CMD_FRAME);

    Decoded d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 1u);
    const std::uint16_t apid = static_cast<std::uint16_t>(((d.bodies[0][0] << 8) | d.bodies[0][1]) & 0x07FF);
    EXPECT_EQ(apid, STAR_APID_PAYLOAD_COMMAND);
    EXPECT_FALSE(star_payload_apid_allowed_pay_to_bus(apid));

    model.request_status();
    d = decode_all(model.drain(UNLIMITED));
    ASSERT_EQ(d.bodies.size(), 1u);
    EXPECT_EQ(d.bodies[0][1], STAR_APID_PAYLOAD_TELEMETRY);
}

TEST(PayObcLinkModel, ResetRestoresKnownStateAndSeed)
{
    PayObcLinkModel model(0);
    Bytes corrupt = CMD_FRAME;
    corrupt.back() ^= 0x01;
    feed(model, concat(corrupt, CMD_FRAME));
    model.corrupt_next_crc();
    model.split_next_frame(3);
    model.double_next_telemetry();
    model.bad_length_next_packet();
    model.bad_apid_next_packet();
    model.receive(CMD2_FRAME.data(), 11); /* leave the decoder mid-frame */
    ASSERT_GT(model.pending_tx_bytes(), 0u);

    model.reset(0x1234);
    EXPECT_EQ(model.pending_tx_bytes(), 0u);
    EXPECT_EQ(model.counters().frames_bad_crc, 0u);
    EXPECT_EQ(model.counters().commands_accepted, 0u);
    EXPECT_EQ(model.next_sequence_count(), 0x1234u);

    /* No fault survives the reset and the partial frame is discarded */
    model.request_status();
    EXPECT_EQ(model.drain(UNLIMITED), REQ_STATUS_SEED_FRAME);

    model.reset(0);
    feed(model, CMD_FRAME);
    EXPECT_EQ(model.drain(UNLIMITED), RSP0_FRAME);
}
