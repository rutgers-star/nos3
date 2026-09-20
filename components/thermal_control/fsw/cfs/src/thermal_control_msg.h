/*******************************************************************************
** File: thermal_control_msg.h
**
** Purpose:
**  Define Thermal Control application commands and telemetry messages
**
*******************************************************************************/
#ifndef _THERMAL_CONTROL_MSG_H_
#define _THERMAL_CONTROL_MSG_H_

#include "cfe.h"

/*
** Ground Command Codes
*/
#define THERMAL_NOOP_CC                0
#define THERMAL_RESET_COUNTERS_CC      1
#define THERMAL_ENABLE_CC              2
#define THERMAL_DISABLE_CC             3
#define THERMAL_SET_THRESHOLDS_CC      4
#define THERMAL_HEATER_OVERRIDE_ON_CC  5
#define THERMAL_HEATER_OVERRIDE_OFF_CC 6

/*
** Telemetry Request Command Codes
*/
#define THERMAL_REQ_HK_TLM 0

/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;

} THERMAL_NoArgs_cmd_t;

/*
** Set thresholds command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    float                   LowThreshold;   /* Temperature to turn heater ON (°C) */
    float                   HighThreshold;  /* Temperature to turn heater OFF (°C) */

} THERMAL_SetThresholds_cmd_t;

/*
** Housekeeping telemetry type definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;

    uint8  CommandErrorCount;
    uint8  CommandCount;

    uint8  ControlEnabled;      /* 1=enabled, 0=disabled */
    uint8  ControlState;        /* ThermalState_t enum value */
    uint8  HeaterState;         /* 1=ON, 0=OFF */
    uint8  HeaterEpsSwitch;     /* EPS switch number controlling heater */

    float  CurrentTemperature;  /* Current temperature reading (°C) */
    float  TempLowThreshold;    /* Turn heater ON below this */
    float  TempHighThreshold;   /* Turn heater OFF above this */

    uint32 HeaterOnCount;       /* Number of times heater turned ON */
    uint32 HeaterOffCount;      /* Number of times heater turned OFF */

} __attribute__((packed)) THERMAL_Hk_tlm_t;
#define THERMAL_HK_TLM_LNGTH sizeof(THERMAL_Hk_tlm_t)

#endif /* _THERMAL_CONTROL_MSG_H_ */
