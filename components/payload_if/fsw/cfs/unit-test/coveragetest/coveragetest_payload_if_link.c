/*
** File: coveragetest_payload_if_link.c
**
** Purpose:
** Tests the payload-link decode behavior used by PAYLOAD_IF_RxTask:
** a frame split across multiple reads, and two frames delivered together.
** These exercise plframe_decode_feed directly, matching how RxTask
** feeds it one byte at a time.
*/

#include <string.h>
#include "utassert.h"
#include "uttest.h"
#include "payload_link/frame.h"

/* Build a minimal valid CCSDS-shaped body: 6-byte header + 1 data byte = 7 bytes */
static void build_test_body(uint8_t *body, size_t *body_len)
{
    body[0] = 0x00; /* APID high bits */
    body[1] = 0x10; /* APID = 0x010 */
    body[2] = 0x00; /* sequence */
    body[3] = 0x00;
    body[4] = 0x00; /* length field: (7 - 7) = 0 */
    body[5] = 0x00;
    body[6] = 0xAB; /* one data byte */
    *body_len = 7;
}

void Test_Frame_SplitAcrossReads(void)
{
    uint8_t body[7];
    size_t  body_len;
    uint8_t frame[PL_MAX_FRAME_LEN];
    size_t  frame_len;
    plframe_decode_ctx_t ctx;
    plframe_decode_result_t result = PL_DECODE_NEED_MORE;
    size_t i;

    build_test_body(body, &body_len);
    frame_len = plframe_encode(body, body_len, frame);
    UtAssert_True(frame_len > 0, "Encode succeeded");

    plframe_decode_init(&ctx);

    /* Feed all bytes except the last one -- should still need more */
    for (i = 0; i < frame_len - 1; i++)
    {
        result = plframe_decode_feed(&ctx, frame[i]);
    }
    UtAssert_INT32_EQ(result, PL_DECODE_NEED_MORE);

    /* Feed the final byte -- should now complete */
    result = plframe_decode_feed(&ctx, frame[frame_len - 1]);
    UtAssert_INT32_EQ(result, PL_DECODE_OK);
    UtAssert_INT32_EQ(ctx.body_len, body_len);
    UtAssert_True(memcmp(ctx.body, body, body_len) == 0, "Decoded body matches original");
}

void Test_Frame_TwoBackToBack(void)
{
    uint8_t body[7];
    size_t  body_len;
    uint8_t frame[PL_MAX_FRAME_LEN];
    size_t  frame_len;
    plframe_decode_ctx_t ctx;
    plframe_decode_result_t result;
    size_t i;
    int     ok_count = 0;

    build_test_body(body, &body_len);
    frame_len = plframe_encode(body, body_len, frame);

    plframe_decode_init(&ctx);

    /* Feed the same encoded frame twice in a row, byte by byte */
    for (int rep = 0; rep < 2; rep++)
    {
        for (i = 0; i < frame_len; i++)
        {
            result = plframe_decode_feed(&ctx, frame[i]);
            if (result == PL_DECODE_OK)
            {
                ok_count++;
                UtAssert_True(memcmp(ctx.body, body, body_len) == 0, "Decoded body matches on repeat");
            }
        }
    }

    UtAssert_INT32_EQ(ok_count, 2);
}

void Test_Frame_GarbageBeforeSync(void)
{
    uint8_t body[7];
    size_t  body_len;
    uint8_t frame[PL_MAX_FRAME_LEN];
    size_t  frame_len;
    plframe_decode_ctx_t ctx;
    plframe_decode_result_t result = PL_DECODE_NEED_MORE;
    uint8_t garbage[5] = {0x00, 0xFF, 0x12, 0x34, 0x56};
    size_t  i;
    int     ok_seen = 0;

    build_test_body(body, &body_len);
    frame_len = plframe_encode(body, body_len, frame);

    plframe_decode_init(&ctx);

    /* Feed junk bytes first -- decoder must not accept them as a frame */
    for (i = 0; i < sizeof(garbage); i++)
    {
        result = plframe_decode_feed(&ctx, garbage[i]);
        UtAssert_True(result != PL_DECODE_OK, "Garbage never produces a false OK");
    }

    /* Now feed a real, valid frame -- decoder must recover and find it */
    for (i = 0; i < frame_len; i++)
    {
        result = plframe_decode_feed(&ctx, frame[i]);
        if (result == PL_DECODE_OK)
        {
            ok_seen = 1;
            UtAssert_True(memcmp(ctx.body, body, body_len) == 0, "Recovered body matches original");
        }
    }
    UtAssert_True(ok_seen == 1, "Decoder recovered a valid frame after garbage");
}

void Test_Frame_BadCrcThenValid(void)
{
    uint8_t body[7];
    size_t  body_len;
    uint8_t frame[PL_MAX_FRAME_LEN];
    size_t  frame_len;
    plframe_decode_ctx_t ctx;
    plframe_decode_result_t result;
    size_t  i;
    int     bad_crc_seen = 0;
    int     ok_seen      = 0;

    build_test_body(body, &body_len);
    frame_len = plframe_encode(body, body_len, frame);

    /* Corrupt the last byte (part of the CRC) to force a bad-CRC result */
    frame[frame_len - 1] ^= 0xFF;

    plframe_decode_init(&ctx);
    for (i = 0; i < frame_len; i++)
    {
        result = plframe_decode_feed(&ctx, frame[i]);
        if (result == PL_DECODE_BAD_CRC)
        {
            bad_crc_seen = 1;
        }
    }
    UtAssert_True(bad_crc_seen == 1, "Corrupted frame reported as bad CRC");

    /* Re-encode a fresh, uncorrupted frame and confirm the same decoder recovers */
    frame_len = plframe_encode(body, body_len, frame);
    for (i = 0; i < frame_len; i++)
    {
        result = plframe_decode_feed(&ctx, frame[i]);
        if (result == PL_DECODE_OK)
        {
            ok_seen = 1;
            UtAssert_True(memcmp(ctx.body, body, body_len) == 0, "Body matches after recovery");
        }
    }
    UtAssert_True(ok_seen == 1, "Decoder accepts next valid frame after bad CRC, without restart");
}

void UtTest_Setup(void)
{
    UtTest_Add(Test_Frame_SplitAcrossReads, NULL, NULL, "Test_Frame_SplitAcrossReads");
    UtTest_Add(Test_Frame_TwoBackToBack, NULL, NULL, "Test_Frame_TwoBackToBack");
    UtTest_Add(Test_Frame_GarbageBeforeSync, NULL, NULL, "Test_Frame_GarbageBeforeSync");
    UtTest_Add(Test_Frame_BadCrcThenValid, NULL, NULL, "Test_Frame_BadCrcThenValid");
}
