/*******************************************************************************
** File:
**   syn_msg.h
**
** Purpose:
**  Define SYN application commands and telemetry messages
**
*******************************************************************************/
#ifndef _SYN_MSG_H_
#define _SYN_MSG_H_

#include "cfe.h"


/*
** Ground Command Codes
*/
#define SYN_NOOP_CC                 0
#define SYN_RESET_COUNTERS_CC       1
#define SYN_RESET_CC                5
#define SYN_PRIO_CC                 6
#define SYN_GET_PDATA_CC            7
#define SYN_DISP_PDATA_CC           8
#define SYN_CONFIG_DL_CC            9
#define SYN_CONFIG_ALPHA_CC         10
#define SYN_ADD_DATA_CC             11

/* 
** Telemetry Request Command Codes
** TODO: Add additional commands required by the specific component
*/
#define SYN_REQ_HK_TLM              0


/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} SYN_NoArgs_cmd_t;


/*
** SYN write configuration command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint32   DeviceCfg;

} SYN_Config_cmd_t;

/*
** SYN housekeeping type definition
*/
typedef struct 
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8   CommandErrorCount;
    uint8   CommandCount;

} __attribute__((packed)) SYN_Hk_tlm_t;
#define SYN_HK_TLM_LNGTH sizeof ( SYN_Hk_tlm_t )

#endif /* _SYN_MSG_H_ */
