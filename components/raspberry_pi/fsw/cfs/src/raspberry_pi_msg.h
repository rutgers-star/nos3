/*******************************************************************************
** File:
**   raspberry_pi_msg.h
**
** Purpose:
**  Define RASPBERRY_PI application commands and telemetry messages
**
*******************************************************************************/
#ifndef _RASPBERRY_PI_MSG_H_
#define _RASPBERRY_PI_MSG_H_

#include "cfe.h"
#include "raspberry_pi_device.h"

/*
** Ground Command Codes
** TODO: Add additional commands required by the specific component
*/
#define RASPBERRY_PI_NOOP_CC           0
#define RASPBERRY_PI_RESET_COUNTERS_CC 1
#define RASPBERRY_PI_ENABLE_CC         2
#define RASPBERRY_PI_DISABLE_CC        3
#define RASPBERRY_PI_CONFIG_CC         4

/*
** Telemetry Request Command Codes
** TODO: Add additional commands required by the specific component
*/
#define RASPBERRY_PI_REQ_HK_TLM   0
#define RASPBERRY_PI_REQ_DATA_TLM 1

/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} RASPBERRY_PI_NoArgs_cmd_t;

/*
** RASPBERRY_PI write configuration command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint32                  DeviceCfg;

} RASPBERRY_PI_Config_cmd_t;

/*
** RASPBERRY_PI device telemetry definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    RASPBERRY_PI_Device_Data_tlm_t  Raspberry_pi;

    /* TODO: This is specific to the raspberry_pi application, remove if using template generator */
    uint16 PassNumber;
    uint8  RegionStatus;

} __attribute__((packed)) RASPBERRY_PI_Device_tlm_t;
#define RASPBERRY_PI_DEVICE_TLM_LNGTH sizeof(RASPBERRY_PI_Device_tlm_t)

/*
** RASPBERRY_PI housekeeping type definition
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
    RASPBERRY_PI_Device_HK_tlm_t DeviceHK;

} __attribute__((packed)) RASPBERRY_PI_Hk_tlm_t;
#define RASPBERRY_PI_HK_TLM_LNGTH sizeof(RASPBERRY_PI_Hk_tlm_t)

#endif /* _RASPBERRY_PI_MSG_H_ */
