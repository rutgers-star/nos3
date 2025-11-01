/*******************************************************************************
** File: thermal_control_app.h
**
** Purpose:
**   This file contains the source code for the Thermal Control application.
**   This app monitors TMP100 temperature sensor and controls the heater
**   via EPS switch to maintain temperature within acceptable range.
**
*******************************************************************************/

#ifndef _THERMAL_CONTROL_APP_H_
#define _THERMAL_CONTROL_APP_H_

/*
** Required header files
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include "thermal_control_events.h"
#include "thermal_control_msg.h"
#include "thermal_control_version.h"

/*
** Thermal control thresholds and parameters
*/
#define THERMAL_TEMP_LOW_THRESHOLD   20.0  /* Turn heater ON below this (°C) */
#define THERMAL_TEMP_HIGH_THRESHOLD  25.0  /* Turn heater OFF above this (°C) */
#define THERMAL_HEATER_EPS_SWITCH    1     /* EPS switch number controlling heater */

/*
** Thermal control states
*/
typedef enum
{
    THERMAL_STATE_DISABLED = 0,
    THERMAL_STATE_IDLE,
    THERMAL_STATE_HEATING,
    THERMAL_STATE_COOLING
} ThermalState_t;

/*
** Type definition (Thermal Control app global data)
*/
typedef struct
{
    /*
    ** Command interface counters
    */
    uint8 CmdCounter;
    uint8 ErrCounter;

    /*
    ** Housekeeping telemetry packet
    */
    THERMAL_Hk_tlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    CFE_ES_RunStatus_Enum_t RunStatus;

    /*
    ** Operational data (not reported in housekeeping)
    */
    CFE_SB_PipeId_t CmdPipe;

    /*
    ** Initialization data (not reported in housekeeping)
    */
    char   PipeName[16];
    uint16 PipeDepth;

    CFE_EVS_BinFilter_t EventFilters[THERMAL_EVENT_COUNTS];
    CFE_SB_Buffer_t     *MsgPtr;

    /*
    ** Thermal control state and data
    */
    ThermalState_t State;
    double         CurrentTemperature;
    bool           HeaterState;
    bool           ControlEnabled;
    double         TempLowThreshold;
    double         TempHighThreshold;
    uint8          HeaterEpsSwitch;
    uint32         HeaterOnCount;
    uint32         HeaterOffCount;

} THERMAL_AppData_t;

/*
** Exported Data
*/
extern THERMAL_AppData_t THERMAL_AppData;

/*
** Exported Functions
*/

/************************************************************************
** Function prototypes
*************************************************************************/

void THERMAL_AppMain(void);
int32 THERMAL_AppInit(void);
void THERMAL_ProcessCommandPacket(void);
void THERMAL_ProcessGroundCommand(void);
void THERMAL_ProcessTelemetry(CFE_SB_Buffer_t *BufPtr);
void THERMAL_ReportHousekeeping(void);
void THERMAL_ResetCounters(void);
bool THERMAL_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, uint16 expected_length);

/* Thermal control functions */
void THERMAL_UpdateControlLoop(double temperature);
void THERMAL_CommandHeater(bool turn_on);
void THERMAL_EnableControl(void);
void THERMAL_DisableControl(void);
void THERMAL_SetThresholds(double low, double high);

#endif /* _THERMAL_CONTROL_APP_H_ */
