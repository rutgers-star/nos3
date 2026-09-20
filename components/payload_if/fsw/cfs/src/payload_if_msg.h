/*******************************************************************************
** File:
**   payload_if_msg.h
**
** Purpose:
**  Define PAYLOAD_IF application commands and telemetry messages
**
*******************************************************************************/
#ifndef _PAYLOAD_IF_MSG_H_
#define _PAYLOAD_IF_MSG_H_

#include "cfe.h"
#include "payload_if_device.h"

/*
** Ground Command Codes
** TODO: Add additional commands required by the specific component
*/
#define PAYLOAD_IF_NOOP_CC           0
#define PAYLOAD_IF_RESET_COUNTERS_CC 1
#define PAYLOAD_IF_ENABLE_CC         2
#define PAYLOAD_IF_DISABLE_CC        3
#define PAYLOAD_IF_CONFIG_CC         4

/*
** Telemetry Request Command Codes
** TODO: Add additional commands required by the specific component
*/
#define PAYLOAD_IF_REQ_HK_TLM   0
#define PAYLOAD_IF_REQ_DATA_TLM 1

/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} PAYLOAD_IF_NoArgs_cmd_t;

/*
** PAYLOAD_IF write configuration command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint32                  DeviceCfg;

} PAYLOAD_IF_Config_cmd_t;

/*
** PAYLOAD_IF device telemetry definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    PAYLOAD_IF_Device_Data_tlm_t  Payload_if;

    /* TODO: This is specific to the payload_if application, remove if using template generator */
    uint16 PassNumber;
    uint8  RegionStatus;

} __attribute__((packed)) PAYLOAD_IF_Device_tlm_t;
#define PAYLOAD_IF_DEVICE_TLM_LNGTH sizeof(PAYLOAD_IF_Device_tlm_t)

/*
** PAYLOAD_IF housekeeping type definition
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
    PAYLOAD_IF_Device_HK_tlm_t DeviceHK;

} __attribute__((packed)) PAYLOAD_IF_Hk_tlm_t;
#define PAYLOAD_IF_HK_TLM_LNGTH sizeof(PAYLOAD_IF_Hk_tlm_t)

#endif /* _PAYLOAD_IF_MSG_H_ */
