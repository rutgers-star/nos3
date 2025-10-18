/*******************************************************************************
** File:
**   mgr_msg.h
**
** Purpose:
**  Define MGR application commands and telemetry messages
**
*******************************************************************************/
#ifndef _MGR_MSG_H_
#define _MGR_MSG_H_

#include "cfe.h"

/*
** Ground Command Codes
*/
#define MGR_NOOP_CC           0
#define MGR_RESET_COUNTERS_CC 1
#define MGR_SET_MODE_CC       2
#define MGR_REBOOT_PREP_CC    3

#define MGR_SCI_PASS_INC_CC      4
#define MGR_SCI_PASS_RESET_CC    5
#define MGR_SET_AK_CC            6
#define MGR_SET_CONUS_CC         7
#define MGR_SET_HI_CC            8
#define MGR_UPDATE_SCI_STATUS_CC 9

/*
** Telemetry Request Command Codes
*/
#define MGR_REQ_HK_TLM 0

/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} MGR_NoArgs_cmd_t;

/*
** Set mode command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;
    uint8                   U8;

} MGR_U8_cmd_t;

/*
** MGR housekeeping type definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8                     CommandErrorCount;
    uint8                     CommandCount;
    uint8                     SpacecraftMode;
    uint16                    BootCounter;
    uint16                    AnomRebootCtr;
    int64                     TimeTics;
    uint8                     ScienceStatus;
    uint16                    SciPassCount;
    uint8                     AkConfig;
    uint8                     ConusConfig;
    uint8                     HiConfig;

} __attribute__((packed)) MGR_Hk_tlm_t;
#define MGR_HK_TLM_LNGTH sizeof(MGR_Hk_tlm_t)

#endif /* _MGR_MSG_H_ */
