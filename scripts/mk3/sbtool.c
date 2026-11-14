/*
 * sbtool: command and telemetry over the local cFS lab apps, run on the busOBC.
 *
 * The SDR MK3 has no ground link on the bench, so this talks to CI_LAB and
 * TO_LAB over the loopback interface from the board's own shell:
 *
 *   sbtool cmd MID CC [HEXBYTES]   send a command packet to CI_LAB
 *   sbtool to-enable [IP]          tell TO_LAB to send telemetry to IP (default 127.0.0.1)
 *   sbtool tlm SECONDS             print the telemetry TO_LAB sends, for SECONDS
 *   sbtool payobc SECONDS [FILE]   print only the packets received from the PayOBC, decoded,
 *                                  and append each one in full hex to FILE
 *   sbtool bridge                  carry command and telemetry packets over stdin and stdout,
 *                                  so a ground system can reach cFS through the debug console
 *
 * Packets use the cFE v1 headers (6 byte primary header; 2 byte command
 * secondary header; 16 byte telemetry header including the spare bytes).
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define CI_LAB_PORT 5012 /* CI_LAB_BASE_UDP_PORT */
#define TO_LAB_PORT 5013 /* cfgTLM_PORT */

#define TO_LAB_CMD_MID          0x18E8
#define TO_LAB_OUTPUT_ENABLE_CC 2

#define PAYLOAD_IF_HK_TLM_MID     0x0860
#define PAYLOAD_IF_PAYOBC_TLM_MID 0x0862
#define CFE_EVS_LONG_EVENT_MID    0x0808

#define CMD_HDR_LEN 8
#define TLM_HDR_LEN 16
#define MAX_PACKET  1024

static int send_command(uint16_t mid, uint8_t cc, const uint8_t *data, size_t data_len)
{
    uint8_t            pkt[MAX_PACKET];
    size_t             len = CMD_HDR_LEN + data_len;
    uint8_t            sum = 0xFF;
    struct sockaddr_in to;
    int                fd;
    size_t             i;

    if (len > sizeof(pkt))
    {
        fprintf(stderr, "command too long\n");
        return -1;
    }

    pkt[0] = (uint8_t)(mid >> 8);
    pkt[1] = (uint8_t)mid;
    pkt[2] = 0xC0; /* unsegmented, sequence count 0 */
    pkt[3] = 0x00;
    pkt[4] = (uint8_t)((len - 7) >> 8);
    pkt[5] = (uint8_t)(len - 7);
    pkt[6] = cc & 0x7F;
    pkt[7] = 0;
    memcpy(pkt + CMD_HDR_LEN, data, data_len);

    /* cFE checksum: the XOR of every byte, checksum included, is 0xFF */
    for (i = 0; i < len; i++)
    {
        sum ^= pkt[i];
    }
    pkt[7] = sum;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }
    memset(&to, 0, sizeof(to));
    to.sin_family      = AF_INET;
    to.sin_port        = htons(CI_LAB_PORT);
    to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (sendto(fd, pkt, len, 0, (struct sockaddr *)&to, sizeof(to)) != (ssize_t)len)
    {
        perror("sendto");
        close(fd);
        return -1;
    }
    close(fd);

    printf("sent MID 0x%04X CC %u, %zu bytes:", mid, cc, len);
    for (i = 0; i < len; i++)
    {
        printf(" %02X", pkt[i]);
    }
    printf("\n");
    return 0;
}

static int parse_hex(const char *text, uint8_t *out, size_t max, size_t *out_len)
{
    size_t n = 0;

    while (*text != '\0')
    {
        unsigned int byte;

        if (*text == ' ' || *text == ':' || *text == ',')
        {
            text++;
            continue;
        }
        if (n >= max || sscanf(text, "%2x", &byte) != 1 || text[1] == '\0')
        {
            return -1;
        }
        out[n++] = (uint8_t)byte;
        text += 2;
    }
    *out_len = n;
    return 0;
}

static uint32_t be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static float be_float(const uint8_t *p)
{
    uint32_t bits = be32(p);
    float    value;

    memcpy(&value, &bits, sizeof(value));
    return value;
}

static const char *opcode_name(uint8_t opcode)
{
    switch (opcode)
    {
        case 0x01: return "grant DCE authority";
        case 0x02: return "revoke DCE authority";
        case 0x03: return "time sync ping";
        case 0x20: return "run experiment";
        case 0x21: return "abort experiment";
        case 0xFF: return "emergency halt";
        default:   return "unknown opcode";
    }
}

/*
** Decode one PayOBC CCSDS packet (as carried in MID 0x0862), using the data
** layouts the PayOBC software sends (PiDaemon payload_app, ICD RevB section 5).
*/
static void print_payobc(const uint8_t *p, size_t len)
{
    uint16_t       apid     = (uint16_t)(((p[0] << 8) | p[1]) & 0x07FF);
    unsigned       seq      = ((p[2] << 8) | p[3]) & 0x3FFF;
    size_t         data_len = len - 6;
    const uint8_t *d        = p + 6;
    size_t         i;

    printf("  PayOBC APID 0x%03X seq %5u: ", apid, seq);
    switch (apid)
    {
        case 0x011:
            if (data_len >= 18)
            {
                printf("HEARTBEAT met=%llu ms state=0x%02X experiment=%u cpu_temp=%.1f C storage=%u MB",
                       ((unsigned long long)be32(d) << 32) | be32(d + 4), d[8], d[9], be_float(d + 10), be32(d + 14));
                return;
            }
            break;

        case 0x012:
            if (data_len >= 2)
            {
                printf("COMMAND RESPONSE to 0x%02X (%s): %s", d[0], opcode_name(d[0]),
                       d[1] == 0 ? "ACK" : (d[1] == 1 ? "NACK" : "status?"));
                return;
            }
            break;

        case 0x021:
            if (data_len >= 1 && (d[0] == 0xA0 || d[0] == 0xA1))
            {
                printf("ADCS REQUEST: %s DCE authority", d[0] == 0xA0 ? "request" : "relinquish");
                return;
            }
            if (data_len >= 33)
            {
                printf("ADCS REQUEST: %s q=[%.3f %.3f %.3f %.3f] rates=[%.3f %.3f %.3f] for %u ms",
                       d[0] == 0x01 ? "target orientation" : (d[0] == 0x02 ? "target spin rate" : "type?"),
                       be_float(d + 1), be_float(d + 5), be_float(d + 9), be_float(d + 13), be_float(d + 17),
                       be_float(d + 21), be_float(d + 25), be32(d + 29));
                return;
            }
            break;

        case 0x030:
        case 0x031:
        case 0x032:
            if (data_len >= 14)
            {
                printf("BULK %s transfer %u chunk %u of %u, %u bytes",
                       apid == 0x030 ? "calculation data" : (apid == 0x031 ? "force-torque data" : "video"), be32(d),
                       be32(d + 4) + 1, be32(d + 8), (d[12] << 8) | d[13]);
                return;
            }
            break;

        case 0x035:
            if (data_len >= 5)
            {
                printf("BULK RESPONSE transfer %u: %s, %zu missing chunk(s)", be32(d), d[4] == 0 ? "ACK" : "NACK",
                       (data_len - 5) / 4);
                return;
            }
            break;

        default:
            break;
    }

    printf("%zu data bytes:", data_len);
    for (i = 0; i < data_len && i < 32; i++)
    {
        printf(" %02X", d[i]);
    }
}

static int is_payobc_packet(const uint8_t *pkt, size_t len)
{
    return ((pkt[0] << 8) | pkt[1]) == PAYLOAD_IF_PAYOBC_TLM_MID && len >= TLM_HDR_LEN + 7;
}

static void print_packet(const uint8_t *pkt, size_t len)
{
    uint16_t mid = (uint16_t)((pkt[0] << 8) | pkt[1]);
    size_t   i;

    printf("MID 0x%04X len %3zu", mid, len);

    if (mid == PAYLOAD_IF_HK_TLM_MID && len >= TLM_HDR_LEN + 5)
    {
        const uint8_t *p = pkt + TLM_HDR_LEN;
        /* DeviceCount counts a successful enable as well as every accepted frame */
        printf("  PAYLOAD_IF HK: cmd_err=%u cmd=%u dev_err=%u dev_ok=%u enabled=%u", p[0], p[1], p[2], p[3], p[4]);
    }
    else if (is_payobc_packet(pkt, len))
    {
        print_payobc(pkt + TLM_HDR_LEN, len - TLM_HDR_LEN);
    }
    else if (mid == CFE_EVS_LONG_EVENT_MID && len > TLM_HDR_LEN + 32)
    {
        /* AppName[20], EventID, EventType, SpacecraftID, ProcessorID, then the message text */
        printf("  EVENT %.20s: %.122s", (const char *)(pkt + TLM_HDR_LEN), (const char *)(pkt + TLM_HDR_LEN + 32));
    }
    else
    {
        printf(" :");
        for (i = 0; i < len && i < 32; i++)
        {
            printf(" %02X", pkt[i]);
        }
    }
    printf("\n");
    fflush(stdout);
}

/*
** Print telemetry for SECONDS. With payobc_only, print only the packets the
** PayOBC sent, log each one in full hex to log (if not NULL), and finish with
** a count per APID.
*/
static int listen_telemetry(int seconds, int payobc_only, FILE *log)
{
    static unsigned    apid_counts[0x800];
    struct sockaddr_in addr;
    struct timeval     tv = {0, 200000};
    uint8_t            pkt[MAX_PACKET];
    time_t             end = time(NULL) + seconds;
    unsigned           total = 0;
    unsigned           apid;
    int                fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        perror("socket");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(TO_LAB_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        perror("bind");
        close(fd);
        return -1;
    }
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (time(NULL) < end)
    {
        ssize_t n = recv(fd, pkt, sizeof(pkt), 0);
        if (n >= 6)
        {
            if (!payobc_only)
            {
                print_packet(pkt, (size_t)n);
            }
            else if (is_payobc_packet(pkt, (size_t)n))
            {
                const uint8_t *p = pkt + TLM_HDR_LEN;
                ssize_t        i;

                apid = ((p[0] << 8) | p[1]) & 0x07FF;
                apid_counts[apid]++;
                total++;
                print_packet(pkt, (size_t)n);
                if (log != NULL)
                {
                    fprintf(log, "%ld APID 0x%03X", (long)time(NULL), apid);
                    for (i = TLM_HDR_LEN; i < n; i++)
                    {
                        fprintf(log, " %02X", pkt[i]);
                    }
                    fprintf(log, "\n");
                    fflush(log);
                }
            }
        }
        else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        {
            perror("recv");
            break;
        }
    }
    close(fd);

    if (payobc_only)
    {
        printf("received %u PayOBC packet(s) in %d s", total, seconds);
        for (apid = 0; apid < 0x800; apid++)
        {
            if (apid_counts[apid] > 0)
            {
                printf(", APID 0x%03X x%u", apid, apid_counts[apid]);
            }
        }
        printf("\n");
    }
    return 0;
}

static volatile sig_atomic_t bridge_stop = 0;

static void bridge_signal(int sig)
{
    (void)sig;
    bridge_stop = 1;
}

/* A line is "TC <hex>" with one complete CCSDS packet, or "quit". Anything else is ignored,
   so a kernel message on the console cannot turn into a command. */
static void bridge_line(int fd, const char *line)
{
    struct sockaddr_in to;
    uint8_t            pkt[MAX_PACKET];
    size_t             len = 0;

    if (strcmp(line, "quit") == 0)
    {
        bridge_stop = 1;
        return;
    }
    if (strncmp(line, "TC ", 3) != 0 || parse_hex(line + 3, pkt, sizeof(pkt), &len) != 0 || len < 7 ||
        ((size_t)((pkt[4] << 8) | pkt[5]) + 7) != len)
    {
        return;
    }

    memset(&to, 0, sizeof(to));
    to.sin_family      = AF_INET;
    to.sin_port        = htons(CI_LAB_PORT);
    to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (sendto(fd, pkt, len, 0, (struct sockaddr *)&to, sizeof(to)) != (ssize_t)len)
    {
        perror("sendto");
    }
}

/*
** Carry packets over stdin and stdout, one line of hex per packet: "TC <hex>" in, sent to
** CI_LAB, and "TM <hex>" out for each packet from TO_LAB. "quit" ends it. Echo is turned
** off for the duration, so the console carries only these lines.
*/
static int bridge(void)
{
    struct sockaddr_in addr;
    struct sigaction   action;
    struct termios     saved;
    struct termios     quiet;
    uint8_t            pkt[MAX_PACKET];
    char               line[2 * MAX_PACKET + 8];
    size_t             used    = 0;
    int                is_tty  = tcgetattr(STDIN_FILENO, &saved) == 0;
    int                fd      = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        perror("socket");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(TO_LAB_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        perror("bind");
        close(fd);
        return -1;
    }

    memset(&action, 0, sizeof(action));
    action.sa_handler = bridge_signal;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);

    if (is_tty)
    {
        quiet = saved;
        quiet.c_lflag &= ~(tcflag_t)(ECHO | ECHONL | ICANON);
        tcsetattr(STDIN_FILENO, TCSANOW, &quiet);
    }

    printf("BRIDGE READY\n");
    fflush(stdout);

    while (!bridge_stop)
    {
        fd_set  ready;
        ssize_t n;

        FD_ZERO(&ready);
        FD_SET(STDIN_FILENO, &ready);
        FD_SET(fd, &ready);
        if (select(fd + 1, &ready, NULL, NULL, NULL) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("select");
            break;
        }

        if (FD_ISSET(fd, &ready))
        {
            n = recv(fd, pkt, sizeof(pkt), 0);
            if (n > 0)
            {
                ssize_t i;

                printf("TM ");
                for (i = 0; i < n; i++)
                {
                    printf("%02X", pkt[i]);
                }
                printf("\n");
                fflush(stdout);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &ready))
        {
            char   ch;
            size_t start = used;

            n = read(STDIN_FILENO, line + used, sizeof(line) - 1 - used);
            if (n <= 0)
            {
                break;
            }
            used += (size_t)n;
            while (start < used)
            {
                ch = line[start];
                if (ch == '\n' || ch == '\r')
                {
                    line[start] = '\0';
                    bridge_line(fd, line);
                    memmove(line, line + start + 1, used - start - 1);
                    used -= start + 1;
                    start = 0;
                }
                else
                {
                    start++;
                }
            }
            if (used == sizeof(line) - 1)
            {
                used = 0; /* an endless line is noise, not a packet */
            }
        }
    }

    if (is_tty)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved);
    }
    close(fd);
    printf("BRIDGE STOPPED\n");
    return 0;
}

static void usage(void)
{
    fprintf(stderr, "usage: sbtool cmd MID CC [HEXBYTES]\n"
                    "       sbtool to-enable [IP]\n"
                    "       sbtool tlm SECONDS\n"
                    "       sbtool payobc SECONDS [FILE]\n"
                    "       sbtool bridge\n");
}

int main(int argc, char **argv)
{
    if (argc >= 4 && strcmp(argv[1], "cmd") == 0)
    {
        uint8_t data[MAX_PACKET];
        size_t  data_len = 0;

        if (argc >= 5 && parse_hex(argv[4], data, sizeof(data) - CMD_HDR_LEN, &data_len) != 0)
        {
            fprintf(stderr, "bad hex data: %s\n", argv[4]);
            return 2;
        }
        return send_command((uint16_t)strtoul(argv[2], NULL, 0), (uint8_t)strtoul(argv[3], NULL, 0), data,
                            data_len) == 0
                   ? 0
                   : 1;
    }
    if (argc >= 2 && strcmp(argv[1], "to-enable") == 0)
    {
        uint8_t ip[16] = {0};
        snprintf((char *)ip, sizeof(ip), "%s", argc >= 3 ? argv[2] : "127.0.0.1");
        return send_command(TO_LAB_CMD_MID, TO_LAB_OUTPUT_ENABLE_CC, ip, sizeof(ip)) == 0 ? 0 : 1;
    }
    if (argc >= 3 && strcmp(argv[1], "tlm") == 0)
    {
        return listen_telemetry(atoi(argv[2]), 0, NULL) == 0 ? 0 : 1;
    }
    if (argc >= 3 && strcmp(argv[1], "payobc") == 0)
    {
        FILE *log = NULL;
        int   status;

        if (argc >= 4 && (log = fopen(argv[3], "a")) == NULL)
        {
            perror(argv[3]);
            return 1;
        }
        status = listen_telemetry(atoi(argv[2]), 1, log);
        if (log != NULL)
        {
            fclose(log);
        }
        return status == 0 ? 0 : 1;
    }
    if (argc >= 2 && strcmp(argv[1], "bridge") == 0)
    {
        return bridge() == 0 ? 0 : 1;
    }
    usage();
    return 2;
}
