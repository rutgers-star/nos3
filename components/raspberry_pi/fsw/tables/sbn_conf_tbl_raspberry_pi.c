/*******************************************************************************
** Purpose:
**  SBN Configuration Table entries for Raspberry Pi peer
**
**  This should be integrated into the main sbn_conf_tbl.c file
**  Add these entries to the appropriate arrays in that file
*******************************************************************************/

/*
** Add to ProtocolModules array in sbn_conf_tbl.c:
*/
{
    .LibFileName = "sbn_tcp.so",  /* Using TCP for reliable connection */
    .LibSymbol = "SBN_TCP_Ops",
    .ProtocolId = SBN_TCP_PROTOCOL_ID
}

/*
** Add to Networks array in sbn_conf_tbl.c:
** This creates a network for external connections
*/
{
    .NetName = "RaspberryPiNet",
    .ProtocolIdx = 0,  /* Index of TCP protocol above */
    .NumPeers = 1,     /* One Raspberry Pi peer */
    .PeerList = {
        {
            .ProcessorID = 100,        /* Unique ProcessorID for Raspberry Pi */
            .SpacecraftID = 0,         /* Same spacecraft */
            .NetNum = 0,               /* Network index */
            .ProtocolConfig = {
                .BufNum = 3,
                .BufSize = 16384,
                .ConnectOut = 0,       /* Accept incoming connection from Pi */
                .ValidCheck = 1,
                .PeerAddr = "",        /* Empty - we're listening */
                .PeerPort = 2234,      /* Port to listen on (match SBN_CLIENT_PORT) */
            }
        }
    }
}

/*
** Alternative configuration if Raspberry Pi should initiate connection:
*/
{
    .NetName = "RaspberryPiNet",
    .ProtocolIdx = 0,  /* Index of TCP protocol */
    .NumPeers = 1,
    .PeerList = {
        {
            .ProcessorID = 100,
            .SpacecraftID = 0,
            .NetNum = 0,
            .ProtocolConfig = {
                .BufNum = 3,
                .BufSize = 16384,
                .ConnectOut = 1,       /* Initiate connection to Pi */
                .ValidCheck = 1,
                .PeerAddr = "192.168.1.100",  /* IP address of Raspberry Pi */
                .PeerPort = 2234,      /* Port on Pi */
            }
        }
    }
}