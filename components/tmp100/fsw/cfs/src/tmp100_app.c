/*******************************************************************************
** File: tmp100_app.c
**
** Purpose:
**   This file contains the source code for the TMP100 application.
**
*******************************************************************************/

/*
** Include Files
*/
#include <arpa/inet.h>
#include "tmp100_app.h"

/*
** Global Data
*/
TMP100_AppData_t TMP100_AppData;

/*
** Application entry point and main process loop
*/
void TMP100_AppMain(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(TMP100_PERF_ID);

    /*
    ** Perform application initialization
    */
    status = TMP100_AppInit();
    if (status != CFE_SUCCESS)
    {
        TMP100_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Main loop
    */
    while (CFE_ES_RunLoop(&TMP100_AppData.RunStatus) == true)
    {
        /*
        ** Performance log exit stamp
        */
        CFE_ES_PerfLogExit(TMP100_PERF_ID);

        /*
        ** Pend on the arrival of the next Software Bus message
        ** Note that this is the standard, but timeouts are available
        */
        status = CFE_SB_ReceiveBuffer((CFE_SB_Buffer_t **)&TMP100_AppData.MsgPtr, TMP100_AppData.CmdPipe,
                                      CFE_SB_PEND_FOREVER);

        /*
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution.
        */
        CFE_ES_PerfLogEntry(TMP100_PERF_ID);

        /*
        ** If the CFE_SB_ReceiveBuffer was successful, then continue to process the command packet
        ** If not, then exit the application in error.
        ** Note that a SB read error should not always result in an app quitting.
        */
        if (status == CFE_SUCCESS)
        {
            TMP100_ProcessCommandPacket();
        }
        else
        {
            CFE_EVS_SendEvent(TMP100_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "TMP100: SB Pipe Read Error = %d",
                              (int)status);
            TMP100_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Disable component, which cleans up the interface, upon exit
    */
    TMP100_Disable();

    /*
    ** Performance log exit stamp
    */
    CFE_ES_PerfLogExit(TMP100_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(TMP100_AppData.RunStatus);
}

/*
** Initialize application
*/
int32 TMP100_AppInit(void)
{
    int32 status = OS_SUCCESS;

    TMP100_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY); /* as default, no filters are used */
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("TMP100: Error registering for event services: 0x%08X\n", (unsigned int)status);
        return status;
    }

    /*
    ** Create the Software Bus command pipe
    */
    status = CFE_SB_CreatePipe(&TMP100_AppData.CmdPipe, TMP100_PIPE_DEPTH, "TMP100_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(TMP100_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "Error Creating SB Pipe,RC=0x%08X",
                          (unsigned int)status);
        return status;
    }

    /*
    ** Subscribe to ground commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TMP100_CMD_MID), TMP100_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(TMP100_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Gnd Cmds, MID=0x%04X, RC=0x%08X", TMP100_CMD_MID,
                          (unsigned int)status);
        return status;
    }

    /*
    ** Subscribe to housekeeping (hk) message requests
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TMP100_REQ_HK_MID), TMP100_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(TMP100_SUB_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X", TMP100_REQ_HK_MID,
                          (unsigned int)status);
        return status;
    }

    /*
    ** Subscribe to MGR HK for Science Pass Information
    ** TODO: This is specific to the tmp100 application, remove if using template generator
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MGR_HK_TLM_MID), TMP100_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(TMP100_SUB_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X", MGR_HK_TLM_MID,
                          (unsigned int)status);
        return status;
    }

    /*
    ** TODO: Subscribe to any other messages here
    */

    /*
    ** Initialize the published HK message - this HK message will contain the
    ** telemetry that has been defined in the TMP100_HkTelemetryPkt for this app.
    */
    CFE_MSG_Init(CFE_MSG_PTR(TMP100_AppData.HkTelemetryPkt.TlmHeader), CFE_SB_ValueToMsgId(TMP100_HK_TLM_MID),
                 TMP100_HK_TLM_LNGTH);

    /*
    ** Initialize the device packet message
    ** This packet is specific to your application
    */
    CFE_MSG_Init(CFE_MSG_PTR(TMP100_AppData.DevicePkt.TlmHeader), CFE_SB_ValueToMsgId(TMP100_DEVICE_TLM_MID),
                 TMP100_DEVICE_TLM_LNGTH);

    /*
    ** TODO: Initialize any other messages that this app will publish
    */

    /*
    ** Always reset all counters during application initialization
    */
    TMP100_ResetCounters();

    /*
    ** Initialize application data
    ** Note that counters are excluded as they were reset in the previous code block
    */
    TMP100_AppData.HkTelemetryPkt.DeviceEnabled          = TMP100_DEVICE_DISABLED;
    TMP100_AppData.HkTelemetryPkt.DeviceHK.TemperatureRaw = 0;
    TMP100_AppData.HkTelemetryPkt.DeviceHK.ConfigRegister  = 0;
    TMP100_AppData.HkTelemetryPkt.DeviceHK.TlowRegister    = 0;
    TMP100_AppData.HkTelemetryPkt.DeviceHK.ThighRegister   = 0;

    /*
     ** Send an information event that the app has initialized.
     ** This is useful for debugging the loading of individual applications.
     */
    status = CFE_EVS_SendEvent(TMP100_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                               "TMP100 App Initialized. Version %d.%d.%d.%d", TMP100_MAJOR_VERSION,
                               TMP100_MINOR_VERSION, TMP100_REVISION, TMP100_MISSION_REV);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("TMP100: Error sending initialization event: 0x%08X\n", (unsigned int)status);
    }
    return status;
}

/*
** Process packets received on the TMP100 command pipe
*/
void TMP100_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(TMP100_AppData.MsgPtr, &MsgId);
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        /*
        ** Ground Commands with command codes fall under the TMP100_CMD_MID (Message ID)
        */
        case TMP100_CMD_MID:
            TMP100_ProcessGroundCommand();
            break;

        /*
        ** Housekeeping requests with command codes fall under the TMP100_REQ_HK_MID (Message ID)
        */
        case TMP100_REQ_HK_MID:
            TMP100_ProcessTelemetryRequest();
            break;

        /*
        ** Update science pass information
        ** TODO: This is specific to the tmp100 application, remove if using template generator
        */
        case MGR_HK_TLM_MID:
            TMP100_ProcessMgrHk();
            break;

        /*
        ** TODO: Add additional message IDs as needed
        */

        /*
        ** All other invalid messages that this app doesn't recognize,
        ** increment the command error counter and log as an error event.
        */
        default:
            /* Increment the command error counter upon receipt of an invalid command packet */
            TMP100_AppData.HkTelemetryPkt.CommandErrorCount++;

            /* Send event failure to the console*/
            CFE_EVS_SendEvent(TMP100_PROCESS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Invalid command packet, MID = 0x%x", CFE_SB_MsgIdToValue(MsgId));
            break;
    }
    return;
}

/*
** Process ground commands
** TODO: Add additional commands required by the specific component
*/
void TMP100_ProcessGroundCommand(void)
{
    CFE_SB_MsgId_t    MsgId       = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /*
    ** MsgId is only needed if the command code is not recognized. See default case
    */
    CFE_MSG_GetMsgId(TMP100_AppData.MsgPtr, &MsgId);

    /*
    ** Ground Commands have a command code (_CC) associated with them
    ** Pull this command code from the message and then process
    */
    CFE_MSG_GetFcnCode(TMP100_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case TMP100_NOOP_CC:
            /*
            ** Verify the command length immediately after CC identification
            */
            if (TMP100_VerifyCmdLength(TMP100_AppData.MsgPtr, sizeof(TMP100_NoArgs_cmd_t)) == OS_SUCCESS)
            {
#ifdef TMP100_CFG_DEBUG
                OS_printf("TMP100: TMP100_NOOP_CC received \n");
#endif

                /* Do any necessary checks, none for a NOOP */

                /* Increment command success or error counter, NOOP can only be successful */
                TMP100_AppData.HkTelemetryPkt.CommandCount++;

                /* Do the action, none for a NOOP */

                /* Increment device success or error counter, none for NOOP as application only */

                /* Send event success or failure to the console, NOOP can only be successful */
                CFE_EVS_SendEvent(TMP100_CMD_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "TMP100: NOOP command received");
            }
            break;

        /*
        ** Reset Counters Command
        */
        case TMP100_RESET_COUNTERS_CC:
            if (TMP100_VerifyCmdLength(TMP100_AppData.MsgPtr, sizeof(TMP100_NoArgs_cmd_t)) == OS_SUCCESS)
            {
#ifdef TMP100_CFG_DEBUG
                OS_printf("TMP100: TMP100_RESET_COUNTERS_CC received \n");
#endif
                TMP100_ResetCounters();
            }
            break;

        /*
        ** Enable Command
        */
        case TMP100_ENABLE_CC:
            if (TMP100_VerifyCmdLength(TMP100_AppData.MsgPtr, sizeof(TMP100_NoArgs_cmd_t)) == OS_SUCCESS)
            {
#ifdef TMP100_CFG_DEBUG
                OS_printf("TMP100: TMP100_ENABLE_CC received \n");
#endif
                TMP100_Enable();
            }
            break;

        /*
        ** Disable Command
        */
        case TMP100_DISABLE_CC:
            if (TMP100_VerifyCmdLength(TMP100_AppData.MsgPtr, sizeof(TMP100_NoArgs_cmd_t)) == OS_SUCCESS)
            {
#ifdef TMP100_CFG_DEBUG
                OS_printf("TMP100: TMP100_DISABLE_CC received \n");
#endif
                TMP100_Disable();
            }
            break;

        /*
        ** Set Configuration Command
        ** Note that this is an example of a command that has additional arguments
        */
        case TMP100_CONFIG_CC:
            if (TMP100_VerifyCmdLength(TMP100_AppData.MsgPtr, sizeof(TMP100_Config_cmd_t)) == OS_SUCCESS)
            {
#ifdef TMP100_CFG_DEBUG
                OS_printf("TMP100: TMP100_CONFIG_CC received \n");
#endif
                TMP100_Configure();
            }
            break;

        /*
        ** TODO: Edit and add more command codes as appropriate for the application
        */

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the command error counter upon receipt of an invalid command */
            TMP100_AppData.HkTelemetryPkt.CommandErrorCount++;

            /* Send invalid command code failure to the console */
            CFE_EVS_SendEvent(TMP100_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x",
                              CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
}

/*
** Process Telemetry Request - Triggered in response to a telemetry request
*/
void TMP100_ProcessTelemetryRequest(void)
{
    CFE_SB_MsgId_t    MsgId       = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /* MsgId is only needed if the command code is not recognized. See default case */
    CFE_MSG_GetMsgId(TMP100_AppData.MsgPtr, &MsgId);

    /* Pull this command code from the message and then process */
    CFE_MSG_GetFcnCode(TMP100_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        case TMP100_REQ_HK_TLM:
            TMP100_ReportHousekeeping();
            break;

        case TMP100_REQ_DATA_TLM:
            TMP100_ReportDeviceTelemetry();
            break;

        /*
        ** TODO: Edit, add, or remove telemetry request codes appropriate for the application
        */

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            TMP100_AppData.HkTelemetryPkt.CommandErrorCount++;

            /* Send invalid command code failure to the console */
            CFE_EVS_SendEvent(TMP100_DEVICE_TLM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x",
                              CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
}

/*
** Report Application Housekeeping
*/
void TMP100_ReportHousekeeping(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (TMP100_AppData.HkTelemetryPkt.DeviceEnabled == TMP100_DEVICE_ENABLED)
    {
        status = TMP100_ReadAllRegisters(&TMP100_AppData.Tmp100I2C,
                                  (TMP100_Device_HK_tlm_t *)&TMP100_AppData.HkTelemetryPkt.DeviceHK);
        if (status == OS_SUCCESS)
        {
            TMP100_AppData.HkTelemetryPkt.DeviceCount++;
        }
        else
        {
            TMP100_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(TMP100_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Request device HK reported error %d", status);
        }
    }
    /* Intentionally do not report errors if disabled */

    /* Time stamp and publish housekeeping telemetry */
    CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&TMP100_AppData.HkTelemetryPkt);
    CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&TMP100_AppData.HkTelemetryPkt, true);
    return;
}

/*
** Collect and Report Device Telemetry
*/
void TMP100_ReportDeviceTelemetry(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (TMP100_AppData.HkTelemetryPkt.DeviceEnabled == TMP100_DEVICE_ENABLED)
    {
        status = TMP100_RequestData(&TMP100_AppData.Tmp100I2C,
                                    (TMP100_Device_Data_tlm_t *)&TMP100_AppData.DevicePkt.Tmp100);
        if (status == OS_SUCCESS)
        {
            /* Update packet count */
            TMP100_AppData.HkTelemetryPkt.DeviceCount++;

            /* Time stamp and publish data telemetry */
            CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&TMP100_AppData.DevicePkt);
            CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&TMP100_AppData.DevicePkt, true);
        }
        else
        {
            TMP100_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(TMP100_REQ_DATA_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Request device data reported error %d", status);
        }

        /* Check device status and act on error */
        if (TMP100_AppData.HkTelemetryPkt.DeviceHK.TemperatureRaw != 0)
        {
            /* Any bit is an error, halting communication until device power cycled */
            TMP100_Disable();

            /* Send device status error to the console */
            CFE_EVS_SendEvent(TMP100_REQ_DATA_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Request device data reported status error %d",
                              TMP100_AppData.HkTelemetryPkt.DeviceHK.TemperatureRaw);
        }
    }
    /* Intentionally do not report errors if device disabled */
    return;
}

/*
** Ingest science MGR data and save it
** TODO: This is specific to the tmp100 application, remove if using template generator
*/
void TMP100_ProcessMgrHk(void)
{
    MGR_Hk_tlm_t *pMsg = (MGR_Hk_tlm_t *)TMP100_AppData.MsgPtr;

    TMP100_AppData.DevicePkt.PassNumber   = pMsg->SciPassCount;
    TMP100_AppData.DevicePkt.RegionStatus = pMsg->ScienceStatus;
    return;
}

/*
** Reset all global counter variables
*/
void TMP100_ResetCounters(void)
{
    /* Do any necessary checks, none for reset counters */

    /* Increment command success or error counter, omitted as action is to reset */

    /* Do the action, clear all global counter variables */
    TMP100_AppData.HkTelemetryPkt.CommandErrorCount = 0;
    TMP100_AppData.HkTelemetryPkt.CommandCount      = 0;
    TMP100_AppData.HkTelemetryPkt.DeviceErrorCount  = 0;
    TMP100_AppData.HkTelemetryPkt.DeviceCount       = 0;

    /* Increment device success or error counter, none as application only */

    /* Send event success to the console */
    CFE_EVS_SendEvent(TMP100_CMD_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TMP100: RESET counters command received");
    return;
}

/*
** Enable Component
** TODO: Edit for your specific component implementation
*/
void TMP100_Enable(void)
{
    int32                  status = OS_SUCCESS;
    TMP100_Device_HK_tlm_t test_data;

    /* Do any necessary checks, confirm that device is currently disabled */
    if (TMP100_AppData.HkTelemetryPkt.DeviceEnabled == TMP100_DEVICE_DISABLED)
    {
        /* Increment command success counter */
        TMP100_AppData.HkTelemetryPkt.CommandCount++;

        /*
        ** Do the action, initialize hardware interface and set enabled
        */
        /* Initialize I2C for TMP100 */
        TMP100_AppData.Tmp100I2C.handle = TMP100_CFG_I2C_HANDLE;
        TMP100_AppData.Tmp100I2C.addr   = TMP100_CFG_I2C_ADDRESS;
        TMP100_AppData.Tmp100I2C.isOpen = I2C_CLOSED;
        TMP100_AppData.Tmp100I2C.speed  = TMP100_CFG_I2C_SPEED;

        status = i2c_master_init(&TMP100_AppData.Tmp100I2C);

        if (status == OS_SUCCESS)
        {
            /* Verify I2C communication by reading temperature register */
            status = TMP100_ReadTemperature(&TMP100_AppData.Tmp100I2C, &test_data);
        }

        if (status == OS_SUCCESS)
        {
            TMP100_AppData.HkTelemetryPkt.DeviceEnabled = TMP100_DEVICE_ENABLED;

            /* Increment device success counter */
            TMP100_AppData.HkTelemetryPkt.DeviceCount++;

            /* Send device event success to the console */
            CFE_EVS_SendEvent(TMP100_ENABLE_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "TMP100: Device enabled successfully");
        }
        else
        {
            /* Failed to communicate with device, close I2C bus */
            i2c_master_close(&TMP100_AppData.Tmp100I2C);

            /* Increment device error counter */
            TMP100_AppData.HkTelemetryPkt.DeviceErrorCount++;

            /* Send device event failure to the console */
            CFE_EVS_SendEvent(TMP100_UART_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Device I2C initialization/communication error %d", status);
        }
    }
    else
    {
        /* Increment command error count */
        TMP100_AppData.HkTelemetryPkt.CommandErrorCount++;

        /* Send command event failure to the console */
        CFE_EVS_SendEvent(TMP100_ENABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TMP100: Device enable failed, already enabled");
    }
    return;
}

/*
** Disable Component
*/
void TMP100_Disable(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is currently enabled */
    if (TMP100_AppData.HkTelemetryPkt.DeviceEnabled == TMP100_DEVICE_ENABLED)
    {
        /* Increment command success counter */
        TMP100_AppData.HkTelemetryPkt.CommandCount++;

        /* Close I2C hardware interface */
        status = i2c_master_close(&TMP100_AppData.Tmp100I2C);

        if (status == OS_SUCCESS)
        {
            TMP100_AppData.HkTelemetryPkt.DeviceEnabled = TMP100_DEVICE_DISABLED;

            /* Increment device success counter */
            TMP100_AppData.HkTelemetryPkt.DeviceCount++;

            /* Send device event success to the console */
            CFE_EVS_SendEvent(TMP100_DISABLE_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "TMP100: Device disabled successfully");
        }
        else
        {
            /* Increment device error counter */
            TMP100_AppData.HkTelemetryPkt.DeviceErrorCount++;

            /* Send device event failure to the console */
            CFE_EVS_SendEvent(TMP100_UART_CLOSE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Device I2C port close error %d", status);
        }
    }
    else
    {
        /* Increment command error count */
        TMP100_AppData.HkTelemetryPkt.CommandErrorCount++;

        /* Send command event failure to the console */
        CFE_EVS_SendEvent(TMP100_DISABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TMP100: Device disable failed, already disabled");
    }
    return;
}

/*
** Configure Component
** TODO: Edit for your specific component implementation
*/
void TMP100_Configure(void)
{
    int32                status        = OS_SUCCESS;
    int32                device_status = OS_SUCCESS;
    TMP100_Config_cmd_t *config_cmd    = (TMP100_Config_cmd_t *)TMP100_AppData.MsgPtr;

    /* Do any necessary checks, confirm that device is currently enabled */
    if (TMP100_AppData.HkTelemetryPkt.DeviceEnabled != TMP100_DEVICE_ENABLED)
    {
        status = OS_ERROR;
        /* Increment command error count */
        TMP100_AppData.HkTelemetryPkt.CommandErrorCount++;

        /* Send event logging failure of check to the console */
        CFE_EVS_SendEvent(TMP100_CMD_CONFIG_EN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TMP100: Configuration command invalid when device disabled");
    }

    /* Do any necessary checks, confirm valid configuration value */
    if (config_cmd->DeviceCfg == 0xFFFFFFFF) // 4294967295
    {
        status = OS_ERROR;
        /* Increment command error count */
        TMP100_AppData.HkTelemetryPkt.CommandErrorCount++;

        /* Send event logging failure of check to the console */
        CFE_EVS_SendEvent(TMP100_CMD_CONFIG_VAL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TMP100: Configuration command with value %u is invalid", config_cmd->DeviceCfg);
    }

    if (status == OS_SUCCESS)
    {
        /* Increment command success counter */
        TMP100_AppData.HkTelemetryPkt.CommandCount++;

        /* Do the action, command device to with a new configuration */
        device_status = TMP100_WriteConfig(&TMP100_AppData.Tmp100I2C, (uint8_t)(config_cmd->DeviceCfg & 0xFF));
        if (device_status == OS_SUCCESS)
        {
            /* Increment device success counter */
            TMP100_AppData.HkTelemetryPkt.DeviceCount++;

            /* Send device event success to the console */
            CFE_EVS_SendEvent(TMP100_CMD_CONFIG_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "TMP100: Configuration command received: %u", config_cmd->DeviceCfg);
        }
        else
        {
            /* Increment device error counter */
            TMP100_AppData.HkTelemetryPkt.DeviceErrorCount++;

            /* Send device event failure to the console */
            CFE_EVS_SendEvent(TMP100_CMD_CONFIG_DEV_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TMP100: Configuration command received: %u", config_cmd->DeviceCfg);
        }
    }
    return;
}

/*
** Verify command packet length matches expected
*/
int32 TMP100_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length)
{
    int32             status        = OS_SUCCESS;
    CFE_SB_MsgId_t    msg_id        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t cmd_code      = 0;
    size_t            actual_length = 0;

    CFE_MSG_GetSize(msg, &actual_length);
    if (expected_length != actual_length)
    {
        CFE_MSG_GetMsgId(msg, &msg_id);
        CFE_MSG_GetFcnCode(msg, &cmd_code);

        CFE_EVS_SendEvent(TMP100_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid msg length: ID = 0x%X,  CC = %d, Len = %ld, Expected = %d",
                          CFE_SB_MsgIdToValue(msg_id), cmd_code, actual_length, expected_length);

        status = OS_ERROR;

        /* Increment the command error counter upon receipt of an invalid command length */
        TMP100_AppData.HkTelemetryPkt.CommandErrorCount++;
    }
    return status;
}
