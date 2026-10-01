/*
** Test for 0001-forward-all-ccsds-space-packets.patch.
**
** Compiles the patched CryptoLib standalone.c (STANDALONE_C) and feeds
** crypto_standalone_spp_telem_or_idle the data field of one TM frame,
** checking which packets it forwards on a loopback UDP socket. Exits non-zero
** on any mismatch. Run with test_spp_forwarding.sh.
*/
#define main standalone_main
#include STANDALONE_C
#undef main
#include <arpa/inet.h>

static int failures = 0;

static void check(int ok, const char *what)
{
    printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok)
    {
        failures++;
    }
}

/* Runs the frame through the extraction loop; returns the forwarded packets' first two bytes */
static int forward(const uint8_t *data, int len, int rx, udp_interface_t *socks, uint16_t *ids, int max_ids)
{
    int32_t  status    = 0;
    uint16_t spp_len   = 0;
    int      remaining = len;
    uint8_t *ptr       = (uint8_t *)data;
    uint8_t  buf[2048];
    int      n = 0;

    while (remaining > 5)
    {
        crypto_standalone_spp_telem_or_idle(&status, ptr, &spp_len, socks, &remaining);
        ptr = &ptr[spp_len];
    }
    while (recv(rx, buf, sizeof(buf), MSG_DONTWAIT) > 0)
    {
        if (n < max_ids)
        {
            ids[n] = (uint16_t)((buf[0] << 8) | buf[1]);
        }
        n++;
    }
    return n;
}

int main(void)
{
    /* PayOBC status: APID 0x011 telemetry without a secondary header, 20 bytes */
    const uint8_t payobc[] = {0x00, 0x11, 0xC0, 0x00, 0x00, 0x0D, 0x01, 0x20, 0x00, 0x01,
                              0x00, 0x01, 0,    0,    0,    0,    0,    0,    0,    0};
    /* cFS housekeeping with a secondary header (0x0860), 12 bytes */
    const uint8_t hk[] = {0x08, 0x60, 0xC0, 0x01, 0x00, 0x05, 1, 2, 3, 4, 5, 6};
    /* Idle packet (APID 0x7FF), 8 bytes */
    const uint8_t idle[] = {0x07, 0xFF, 0xC0, 0x00, 0x00, 0x01, 0x55, 0x55};
    /* Packet whose length field runs past the end of the frame */
    const uint8_t truncated[] = {0x00, 0x11, 0xC0, 0x00, 0x01, 0x00, 0xAA, 0xBB};
    uint8_t       frame[256];
    int           len = 0;
    uint16_t      ids[8];
    int           n;

    memcpy(frame + len, payobc, sizeof(payobc));
    len += sizeof(payobc);
    memcpy(frame + len, hk, sizeof(hk));
    len += sizeof(hk);
    memcpy(frame + len, idle, sizeof(idle));
    len += sizeof(idle);
    memset(frame + len, 0x00, 16); /* idle frame fill */
    len += 16;

    int                rx = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family      = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind(rx, (struct sockaddr *)&a, sizeof(a));
    socklen_t al = sizeof(a);
    getsockname(rx, (struct sockaddr *)&a, &al);

    udp_interface_t socks;
    memset(&socks, 0, sizeof(socks));
    socks.write.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    socks.write.saddr  = a;

    n = forward(frame, len, rx, &socks, ids, 8);
    check(n == 2 && ids[0] == 0x0011 && ids[1] == 0x0860,
          "[0x0011 no secondary header][0x0860 HK][idle][fill]: both packets forwarded in order");

    n = forward(truncated, sizeof(truncated), rx, &socks, ids, 8);
    check(n == 0, "packet longer than the rest of the frame: nothing forwarded, processing stops");

    printf("%s (%d failure(s))\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
