/*******************************************************************************
** File:
**   tmp100_msg.h
**
** Purpose:
**  Define TMP100 application commands and telemetry messages
**
*******************************************************************************/
#ifndef _TMP100_MSG_H_
#define _TMP100_MSG_H_

#include "cfe.h"
#include "tmp100_device.h"

/*
** Ground Command Codes
** TODO: Add additional commands required by the specific component
*/
#define TMP100_NOOP_CC           0
#define TMP100_RESET_COUNTERS_CC 1
#define TMP100_ENABLE_CC         2
#define TMP100_DISABLE_CC        3
#define TMP100_CONFIG_CC         4

/*
** Telemetry Request Command Codes
** TODO: Add additional commands required by the specific component
*/
#define TMP100_REQ_HK_TLM   0
#define TMP100_REQ_DATA_TLM 1

/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} TMP100_NoArgs_cmd_t;

/*
** TMP100 write configuration command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint32                  DeviceCfg;

} TMP100_Config_cmd_t;

/*
** TMP100 device telemetry definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    TMP100_Device_Data_tlm_t  Tmp100;

    /* TODO: This is specific to the tmp100 application, remove if using template generator */
    uint16 PassNumber;
    uint8  RegionStatus;

} __attribute__((packed)) TMP100_Device_tlm_t;
#define TMP100_DEVICE_TLM_LNGTH sizeof(TMP100_Device_tlm_t)

/*
** TMP100 housekeeping type definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8                     CommandErrorCount;
    uint8                     CommandCount;
    uint8                     DeviceErrorCount;
    uint8                     DeviceCount;

    /*
    ** TODO: Edit and add specific telemetry values to this struct
    */
    uint8                  DeviceEnabled;
    TMP100_Device_HK_tlm_t DeviceHK;

} __attribute__((packed)) TMP100_Hk_tlm_t;
#define TMP100_HK_TLM_LNGTH sizeof(TMP100_Hk_tlm_t)

#endif /* _TMP100_MSG_H_ */
