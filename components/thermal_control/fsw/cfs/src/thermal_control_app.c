/*******************************************************************************
** File: thermal_control_app.c
**
** Purpose:
**   This file contains the source code for the Thermal Control application.
**   Implements closed-loop thermal control by monitoring TMP100 temperature
**   sensor and commanding the heater via EPS switch.
**
*******************************************************************************/

/*
** Include Files
*/
#include "thermal_control_app.h"
#include "thermal_control_perfids.h"
#include "thermal_control_msgids.h"

/* Need to include EPS and TMP100 message IDs and structures */
#include "generic_eps_msgids.h"
#include "generic_eps_msg.h"
#include "tmp100_msgids.h"
#include "tmp100_msg.h"

/*
** Global Data
*/
THERMAL_AppData_t THERMAL_AppData;

/*
** Application entry point and main process loop
*/
void THERMAL_AppMain(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(THERMAL_PERF_ID);

    /*
    ** Perform application initialization
    */
    status = THERMAL_AppInit();
    if (status != CFE_SUCCESS)
    {
        THERMAL_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Main loop
    */
    while (CFE_ES_RunLoop(&THERMAL_AppData.RunStatus) == true)
    {
        /*
        ** Performance log exit stamp
        */
        CFE_ES_PerfLogExit(THERMAL_PERF_ID);

        /*
        ** Pend on the arrival of the next Software Bus message
        */
        status = CFE_SB_ReceiveBuffer((CFE_SB_Buffer_t **)&THERMAL_AppData.MsgPtr,
                                      THERMAL_AppData.CmdPipe,
                                      CFE_SB_PEND_FOREVER);

        /*
        ** Performance log entry stamp
        */
        CFE_ES_PerfLogEntry(THERMAL_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            THERMAL_ProcessCommandPacket();
        }
        else
        {
            CFE_EVS_SendEvent(THERMAL_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "THERMAL: SB Pipe Read Error = %d", (int)status);
            THERMAL_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance log exit stamp
    */
    CFE_ES_PerfLogExit(THERMAL_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(THERMAL_AppData.RunStatus);
}

/*
** Initialize application
*/
int32 THERMAL_AppInit(void)
{
    int32 status = OS_SUCCESS;

    THERMAL_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    THERMAL_AppData.CmdCounter = 0;
    THERMAL_AppData.ErrCounter = 0;

    /*
    ** Initialize thermal control state
    */
    THERMAL_AppData.State = THERMAL_STATE_IDLE;
    THERMAL_AppData.CurrentTemperature = 0.0;
    THERMAL_AppData.HeaterState = false;
    THERMAL_AppData.ControlEnabled = true;  /* Start enabled */
    THERMAL_AppData.TempLowThreshold = THERMAL_TEMP_LOW_THRESHOLD;
    THERMAL_AppData.TempHighThreshold = THERMAL_TEMP_HIGH_THRESHOLD;
    THERMAL_AppData.HeaterEpsSwitch = THERMAL_HEATER_EPS_SWITCH;
    THERMAL_AppData.HeaterOnCount = 0;
    THERMAL_AppData.HeaterOffCount = 0;

    /*
    ** Initialize app configuration data
    */
    strcpy(THERMAL_AppData.PipeName, "THERMAL_CMD_PIPE");
    THERMAL_AppData.PipeDepth = 32;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(THERMAL_AppData.EventFilters, THERMAL_EVENT_COUNTS,
                              CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Thermal Control App: Error Registering Events, RC = 0x%08lX\n",
                             (unsigned long)status);
        return (status);
    }

    /*
    ** Create Software Bus message pipe
    */
    status = CFE_SB_CreatePipe(&THERMAL_AppData.CmdPipe, THERMAL_AppData.PipeDepth,
                               THERMAL_AppData.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Thermal Control App: Error creating pipe, RC = 0x%08lX\n",
                             (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(THERMAL_SEND_HK_MID), THERMAL_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Thermal Control App: Error Subscribing to HK request, RC = 0x%08lX\n",
                             (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(THERMAL_CMD_MID), THERMAL_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Thermal Control App: Error Subscribing to Command, RC = 0x%08lX\n",
                             (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to TMP100 housekeeping telemetry
    ** This is where we get temperature readings
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TMP100_HK_TLM_MID), THERMAL_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Thermal Control App: Error Subscribing to TMP100 HK, RC = 0x%08lX\n",
                             (unsigned long)status);
        return (status);
    }

    /*
    ** Initialize housekeeping packet (clear user data area)
    */
    CFE_MSG_Init(&THERMAL_AppData.HkTlm.TlmHeader.Msg, CFE_SB_ValueToMsgId(THERMAL_HK_TLM_MID),
                 THERMAL_HK_TLM_LNGTH);

    CFE_EVS_SendEvent(THERMAL_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "Thermal Control App Initialized. Version %d.%d.%d.%d",
                      THERMAL_MAJOR_VERSION, THERMAL_MINOR_VERSION, THERMAL_REVISION,
                      THERMAL_MISSION_REV);

    CFE_EVS_SendEvent(THERMAL_ENABLED_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "Thermal Control Enabled: Low=%.1f°C, High=%.1f°C, EPS Switch=%d",
                      THERMAL_AppData.TempLowThreshold, THERMAL_AppData.TempHighThreshold,
                      THERMAL_AppData.HeaterEpsSwitch);

    return CFE_SUCCESS;
}

/*
** Process packets received on the command pipe
*/
void THERMAL_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(THERMAL_AppData.MsgPtr, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case THERMAL_CMD_MID:
            THERMAL_ProcessGroundCommand();
            break;

        case THERMAL_SEND_HK_MID:
            THERMAL_ReportHousekeeping();
            break;

        case TMP100_HK_TLM_MID:
            THERMAL_ProcessTelemetry(THERMAL_AppData.MsgPtr);
            break;

        default:
            CFE_EVS_SendEvent(THERMAL_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "THERMAL: invalid command packet, MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}

/*
** Process ground commands
*/
void THERMAL_ProcessGroundCommand(void)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(THERMAL_AppData.MsgPtr, &CommandCode);

    switch (CommandCode)
    {
        case THERMAL_NOOP_CC:
            if (THERMAL_VerifyCmdLength(THERMAL_AppData.MsgPtr, sizeof(THERMAL_NoArgs_cmd_t)))
            {
                THERMAL_AppData.CmdCounter++;
                CFE_EVS_SendEvent(THERMAL_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "THERMAL: NOOP command");
            }
            break;

        case THERMAL_RESET_COUNTERS_CC:
            if (THERMAL_VerifyCmdLength(THERMAL_AppData.MsgPtr, sizeof(THERMAL_NoArgs_cmd_t)))
            {
                THERMAL_ResetCounters();
            }
            break;

        case THERMAL_ENABLE_CC:
            if (THERMAL_VerifyCmdLength(THERMAL_AppData.MsgPtr, sizeof(THERMAL_NoArgs_cmd_t)))
            {
                THERMAL_EnableControl();
                THERMAL_AppData.CmdCounter++;
            }
            break;

        case THERMAL_DISABLE_CC:
            if (THERMAL_VerifyCmdLength(THERMAL_AppData.MsgPtr, sizeof(THERMAL_NoArgs_cmd_t)))
            {
                THERMAL_DisableControl();
                THERMAL_AppData.CmdCounter++;
            }
            break;

        case THERMAL_SET_THRESHOLDS_CC:
            if (THERMAL_VerifyCmdLength(THERMAL_AppData.MsgPtr, sizeof(THERMAL_SetThresholds_cmd_t)))
            {
                THERMAL_SetThresholds_cmd_t *cmd = (THERMAL_SetThresholds_cmd_t *)THERMAL_AppData.MsgPtr;
                THERMAL_SetThresholds(cmd->LowThreshold, cmd->HighThreshold);
                THERMAL_AppData.CmdCounter++;
            }
            break;

        case THERMAL_HEATER_OVERRIDE_ON_CC:
            if (THERMAL_VerifyCmdLength(THERMAL_AppData.MsgPtr, sizeof(THERMAL_NoArgs_cmd_t)))
            {
                THERMAL_CommandHeater(true);
                THERMAL_AppData.CmdCounter++;
                CFE_EVS_SendEvent(THERMAL_HEATER_ON_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "THERMAL: Manual heater override ON");
            }
            break;

        case THERMAL_HEATER_OVERRIDE_OFF_CC:
            if (THERMAL_VerifyCmdLength(THERMAL_AppData.MsgPtr, sizeof(THERMAL_NoArgs_cmd_t)))
            {
                THERMAL_CommandHeater(false);
                THERMAL_AppData.CmdCounter++;
                CFE_EVS_SendEvent(THERMAL_HEATER_OFF_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "THERMAL: Manual heater override OFF");
            }
            break;

        default:
            CFE_EVS_SendEvent(THERMAL_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "THERMAL: invalid command code: CC = %d", CommandCode);
            THERMAL_AppData.ErrCounter++;
            break;
    }
}

/*
** Process TMP100 telemetry and update thermal control loop
*/
void THERMAL_ProcessTelemetry(CFE_SB_MsgPtr_t MsgPtr)
{
    TMP100_Hk_tlm_t *tmp100_hk = (TMP100_Hk_tlm_t *)MsgPtr;

    /* Extract temperature from TMP100 telemetry
    ** The TMP100 stores temperature as a 12-bit value with 0.0625°C resolution
    ** Temperature register format: int16 with centi-degrees
    */
    int16_t temp_register = tmp100_hk->DeviceHK.TemperatureRaw;
    double temperature = (double)temp_register * 0.0625; /* Convert to Celsius */

    /* Update current temperature */
    THERMAL_AppData.CurrentTemperature = temperature;

    /* Run thermal control algorithm if enabled */
    if (THERMAL_AppData.ControlEnabled)
    {
        THERMAL_UpdateControlLoop(temperature);
    }
}

/*
** Thermal control algorithm with hysteresis
*/
void THERMAL_UpdateControlLoop(double temperature)
{
    bool prev_heater_state = THERMAL_AppData.HeaterState;

    switch (THERMAL_AppData.State)
    {
        case THERMAL_STATE_IDLE:
        case THERMAL_STATE_COOLING:
            /* Check if temperature dropped below low threshold */
            if (temperature < THERMAL_AppData.TempLowThreshold)
            {
                /* Too cold - turn heater ON */
                THERMAL_CommandHeater(true);
                THERMAL_AppData.State = THERMAL_STATE_HEATING;

                CFE_EVS_SendEvent(THERMAL_HEATER_ON_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "THERMAL: Temperature %.2f°C < %.2f°C - Heater ON",
                                  temperature, THERMAL_AppData.TempLowThreshold);
            }
            else
            {
                THERMAL_AppData.State = THERMAL_STATE_COOLING;
            }
            break;

        case THERMAL_STATE_HEATING:
            /* Check if temperature rose above high threshold */
            if (temperature > THERMAL_AppData.TempHighThreshold)
            {
                /* Warm enough - turn heater OFF */
                THERMAL_CommandHeater(false);
                THERMAL_AppData.State = THERMAL_STATE_COOLING;

                CFE_EVS_SendEvent(THERMAL_HEATER_OFF_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "THERMAL: Temperature %.2f°C > %.2f°C - Heater OFF",
                                  temperature, THERMAL_AppData.TempHighThreshold);
            }
            break;

        case THERMAL_STATE_DISABLED:
        default:
            /* Do nothing */
            break;
    }

    /* Track state changes for telemetry */
    if (THERMAL_AppData.HeaterState != prev_heater_state)
    {
        if (THERMAL_AppData.HeaterState)
        {
            THERMAL_AppData.HeaterOnCount++;
        }
        else
        {
            THERMAL_AppData.HeaterOffCount++;
        }
    }
}

/*
** Command heater via EPS switch
*/
void THERMAL_CommandHeater(bool turn_on)
{
    GENERIC_EPS_Switch_cmd_t eps_cmd;
    int32 status;

    /* Initialize command message */
    CFE_MSG_Init(&eps_cmd.CmdHeader.Msg, CFE_SB_ValueToMsgId(GENERIC_EPS_CMD_MID),
                 sizeof(GENERIC_EPS_Switch_cmd_t));
    CFE_MSG_SetFcnCode(&eps_cmd.CmdHeader.Msg, GENERIC_EPS_SWITCH_CC);

    /* Set switch number and state */
    eps_cmd.SwitchNumber = THERMAL_AppData.HeaterEpsSwitch;
    eps_cmd.State = turn_on ? 0xAA : 0x00;  /* 0xAA = ON, 0x00 = OFF */

    /* Send command to EPS app */
    status = CFE_SB_TransmitMsg(&eps_cmd.CmdHeader.Msg, true);

    if (status == CFE_SUCCESS)
    {
        THERMAL_AppData.HeaterState = turn_on;
    }
    else
    {
        CFE_EVS_SendEvent(THERMAL_EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "THERMAL: Failed to send EPS command, status = %d", (int)status);
        THERMAL_AppData.ErrCounter++;
    }
}

/*
** Enable thermal control
*/
void THERMAL_EnableControl(void)
{
    THERMAL_AppData.ControlEnabled = true;
    THERMAL_AppData.State = THERMAL_STATE_IDLE;

    CFE_EVS_SendEvent(THERMAL_ENABLED_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "THERMAL: Control Enabled - Low=%.1f°C, High=%.1f°C",
                      THERMAL_AppData.TempLowThreshold, THERMAL_AppData.TempHighThreshold);
}

/*
** Disable thermal control (leaves heater in current state)
*/
void THERMAL_DisableControl(void)
{
    THERMAL_AppData.ControlEnabled = false;
    THERMAL_AppData.State = THERMAL_STATE_DISABLED;

    CFE_EVS_SendEvent(THERMAL_DISABLED_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "THERMAL: Control Disabled");
}

/*
** Set temperature thresholds
*/
void THERMAL_SetThresholds(double low, double high)
{
    /* Validate thresholds */
    if (low >= high)
    {
        CFE_EVS_SendEvent(THERMAL_THRESHOLD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "THERMAL: Invalid thresholds - Low (%.1f) must be < High (%.1f)",
                          low, high);
        THERMAL_AppData.ErrCounter++;
        return;
    }

    if (low < -40.0 || high > 125.0)
    {
        CFE_EVS_SendEvent(THERMAL_THRESHOLD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "THERMAL: Thresholds out of range (-40 to 125°C)");
        THERMAL_AppData.ErrCounter++;
        return;
    }

    THERMAL_AppData.TempLowThreshold = low;
    THERMAL_AppData.TempHighThreshold = high;

    CFE_EVS_SendEvent(THERMAL_THRESHOLD_SET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "THERMAL: Thresholds set - Low=%.1f°C, High=%.1f°C", low, high);
}

/*
** Report housekeeping data
*/
void THERMAL_ReportHousekeeping(void)
{
    /* Update telemetry */
    THERMAL_AppData.HkTlm.CommandErrorCount = THERMAL_AppData.ErrCounter;
    THERMAL_AppData.HkTlm.CommandCount = THERMAL_AppData.CmdCounter;
    THERMAL_AppData.HkTlm.ControlEnabled = THERMAL_AppData.ControlEnabled ? 1 : 0;
    THERMAL_AppData.HkTlm.ControlState = THERMAL_AppData.State;
    THERMAL_AppData.HkTlm.HeaterState = THERMAL_AppData.HeaterState ? 1 : 0;
    THERMAL_AppData.HkTlm.HeaterEpsSwitch = THERMAL_AppData.HeaterEpsSwitch;
    THERMAL_AppData.HkTlm.CurrentTemperature = (float)THERMAL_AppData.CurrentTemperature;
    THERMAL_AppData.HkTlm.TempLowThreshold = (float)THERMAL_AppData.TempLowThreshold;
    THERMAL_AppData.HkTlm.TempHighThreshold = (float)THERMAL_AppData.TempHighThreshold;
    THERMAL_AppData.HkTlm.HeaterOnCount = THERMAL_AppData.HeaterOnCount;
    THERMAL_AppData.HkTlm.HeaterOffCount = THERMAL_AppData.HeaterOffCount;

    CFE_SB_TimeStampMsg(&THERMAL_AppData.HkTlm.TlmHeader.Msg);
    CFE_SB_TransmitMsg(&THERMAL_AppData.HkTlm.TlmHeader.Msg, true);
}

/*
** Reset counters
*/
void THERMAL_ResetCounters(void)
{
    THERMAL_AppData.CmdCounter = 0;
    THERMAL_AppData.ErrCounter = 0;
    THERMAL_AppData.HeaterOnCount = 0;
    THERMAL_AppData.HeaterOffCount = 0;

    CFE_EVS_SendEvent(THERMAL_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "THERMAL: RESET command");
}

/*
** Verify command length
*/
bool THERMAL_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 expected_length)
{
    bool               result       = true;
    CFE_SB_MsgId_t     MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t  FcnCode      = 0;
    size_t             ActualLength = 0;

    CFE_MSG_GetSize(msg, &ActualLength);

    if (expected_length != ActualLength)
    {
        CFE_MSG_GetMsgId(msg, &MsgId);
        CFE_MSG_GetFcnCode(msg, &FcnCode);

        CFE_EVS_SendEvent(THERMAL_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid msg length: ID = 0x%X,  CC = %d, Len = %ld, Expected = %d",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (int)FcnCode,
                          (long)ActualLength, (int)expected_length);

        result = false;
        THERMAL_AppData.ErrCounter++;
    }

    return result;
}
