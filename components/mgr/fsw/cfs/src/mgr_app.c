/*******************************************************************************
** File: mgr_app.c
**
** Purpose:
**   This file contains the source code for the MGR application.
**
*******************************************************************************/

/*
** Include Files
*/
#include <arpa/inet.h>
#include "mgr_app.h"

/*
** Global Data
*/
MGR_AppData_t MGR_AppData;

/*
** Application entry point and main process loop
*/
void MGR_AppMain(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(MGR_PERF_ID);

    /*
    ** Perform application initialization
    */
    status = MGR_AppInit();
    if (status != CFE_SUCCESS)
    {
        MGR_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Main loop
    */
    while (CFE_ES_RunLoop(&MGR_AppData.RunStatus) == true)
    {
        /*
        ** Performance log exit stamp
        */
        CFE_ES_PerfLogExit(MGR_PERF_ID);

        /*
        ** Pend on the arrival of the next Software Bus message
        ** Note that this is the standard, but timeouts are available
        */
        status =
            CFE_SB_ReceiveBuffer((CFE_SB_Buffer_t **)&MGR_AppData.MsgPtr, MGR_AppData.CmdPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution.
        */
        CFE_ES_PerfLogEntry(MGR_PERF_ID);

        /*
        ** If the CFE_SB_ReceiveBuffer was successful, then continue to process the command packet
        ** If not, then exit the application in error.
        ** Note that a SB read error should not always result in an app quitting.
        */
        if (status == CFE_SUCCESS)
        {
            MGR_ProcessCommandPacket();
        }
        else
        {
            CFE_EVS_SendEvent(MGR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "MGR: SB Pipe Read Error = %d", (int)status);
            MGR_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance log exit stamp
    */
    CFE_ES_PerfLogExit(MGR_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(MGR_AppData.RunStatus);
}

/*
** Initialize application
*/
int32 MGR_AppInit(void)
{
    int32 status = OS_SUCCESS;

    MGR_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY); /* as default, no filters are used */
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("MGR: Error registering for event services: 0x%08X\n", (unsigned int)status);
        return status;
    }

    /*
    ** Create the Software Bus command pipe
    */
    status = CFE_SB_CreatePipe(&MGR_AppData.CmdPipe, MGR_PIPE_DEPTH, "MGR_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MGR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "Error Creating SB Pipe,RC=0x%08X",
                          (unsigned int)status);
        return status;
    }

    /*
    ** Subscribe to ground commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MGR_CMD_MID), MGR_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MGR_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Gnd Cmds, MID=0x%04X, RC=0x%08X", MGR_CMD_MID, (unsigned int)status);
        return status;
    }

    /*
    ** Subscribe to housekeeping (hk) message requests
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MGR_REQ_HK_MID), MGR_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MGR_SUB_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X", MGR_REQ_HK_MID,
                          (unsigned int)status);
        return status;
    }

    /*
    ** Initialize the published HK message - this HK message will contain the
    ** telemetry that has been defined in the MGR_HkTelemetryPkt for this app.
    ** Note: Intentionally after RestoreHkFile to ensure TlmHeader correct.
    */
    CFE_MSG_Init(CFE_MSG_PTR(MGR_AppData.HkTelemetryPkt.TlmHeader), CFE_SB_ValueToMsgId(MGR_HK_TLM_MID),
                 MGR_HK_TLM_LNGTH);

    /*
    ** Restore Housekeeping Telemetry from the file system.
    ** If the restore fails for any reason, the data will be cleared.
    ** This function will also set some values such as SpacecraftMode if necessary.
    */
    MGR_RestoreHkFile();

    /*
    ** Always reset all counters during application initialization
    */
    MGR_ResetCounters();

    /*
     ** Send an information event that the app has initialized.
     ** This is useful for debugging the loading of individual applications.
     */
    status = CFE_EVS_SendEvent(MGR_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                               "MGR App Initialized. Version %d.%d.%d.%d", MGR_MAJOR_VERSION, MGR_MINOR_VERSION,
                               MGR_REVISION, MGR_MISSION_REV);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("MGR: Error sending initialization event: 0x%08X\n", (unsigned int)status);
    }
    return status;
}

/*
** Process packets received on the MGR command pipe
*/
void MGR_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(MGR_AppData.MsgPtr, &MsgId);
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        /*
        ** Ground Commands with command codes fall under the MGR_CMD_MID (Message ID)
        */
        case MGR_CMD_MID:
            MGR_ProcessGroundCommand();
            break;

        /*
        ** All other messages, other than ground commands, add to this case statement.
        */
        case MGR_REQ_HK_MID:
            MGR_ProcessTelemetryRequest();
            break;

        /*
        ** All other invalid messages that this app doesn't recognize,
        ** increment the command error counter and log as an error event.
        */
        default:
            MGR_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(MGR_PROCESS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MGR: Invalid command packet, MID = 0x%x", CFE_SB_MsgIdToValue(MsgId));
            break;
    }
    return;
}

/*
** Process ground commands
*/
void MGR_ProcessGroundCommand(void)
{
    CFE_SB_MsgId_t    MsgId       = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /*
    ** MsgId is only needed if the command code is not recognized. See default case
    */
    CFE_MSG_GetMsgId(MGR_AppData.MsgPtr, &MsgId);

    /*
    ** Ground Commands, by definition, have a command code (_CC) associated with them
    ** Pull this command code from the message and then process
    */
    CFE_MSG_GetFcnCode(MGR_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case MGR_NOOP_CC:
            /*
            ** First, verify the command length immediately after CC identification
            ** Note that VerifyCmdLength handles the command and command error counters
            */
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                /* Second, send EVS event on successful receipt ground commands*/
                CFE_EVS_SendEvent(MGR_CMD_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "MGR: NOOP command received");
                /* Third, do the desired command action if applicable, in the case of NOOP it is no operation */
            }
            break;

        /*
        ** Reset Counters Command
        */
        case MGR_RESET_COUNTERS_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(MGR_CMD_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "MGR: RESET counters command received");
                MGR_ResetCounters();
            }
            break;

        /*
        ** Set Mode Command
        */
        case MGR_SET_MODE_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_U8_cmd_t)) == OS_SUCCESS)
            {
                MGR_U8_cmd_t *mode_cmd = (MGR_U8_cmd_t *)MGR_AppData.MsgPtr;
                if ((mode_cmd->U8 < MGR_SAFE_MODE) || (mode_cmd->U8 > MGR_SCIENCE_REBOOT_MODE))
                {
                    CFE_EVS_SendEvent(MGR_CMD_SETMODE_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "MGR: Invalid mode commanded [%d], mode remains [%d]", mode_cmd->U8,
                                      MGR_AppData.HkTelemetryPkt.SpacecraftMode);
                }
                else
                {
                    MGR_AppData.HkTelemetryPkt.SpacecraftMode = mode_cmd->U8;
                    CFE_EVS_SendEvent(MGR_CMD_SETMODE_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "MGR: Set mode command received [%d]", MGR_AppData.HkTelemetryPkt.SpacecraftMode);
                }
            }
            break;

        /*
        ** Reboot Prep Command
        */
        case MGR_REBOOT_PREP_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                if ((MGR_AppData.HkTelemetryPkt.SpacecraftMode == MGR_SCIENCE_MODE) ||
                    (MGR_AppData.HkTelemetryPkt.SpacecraftMode == MGR_SCIENCE_REBOOT_MODE))
                {
                    MGR_AppData.HkTelemetryPkt.SpacecraftMode = MGR_SCIENCE_REBOOT_MODE;
                    CFE_EVS_SendEvent(MGR_CMD_REBOOT_PREP_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "MGR: Reboot prep commanded, mode now SCIENCE_REBOOT_MODE [%d]",
                                      MGR_AppData.HkTelemetryPkt.SpacecraftMode);
                }
                else
                {
                    MGR_AppData.HkTelemetryPkt.SpacecraftMode = MGR_SAFE_REBOOT_MODE;
                    CFE_EVS_SendEvent(MGR_CMD_REBOOT_PREP_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "MGR: Reboot prep commanded, mode now SAFE_REBOOT_MODE [%d]",
                                      MGR_AppData.HkTelemetryPkt.SpacecraftMode);
                }
            }
            break;

        /*
        ** Science Pass Counter Increment
        */
        case MGR_SCI_PASS_INC_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                MGR_AppData.HkTelemetryPkt.SciPassCount = MGR_AppData.HkTelemetryPkt.SciPassCount + 1;
                CFE_EVS_SendEvent(MGR_CMD_RESET_SCIPASS_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "MGR: Increment science pass counter command received, now %d",
                                  MGR_AppData.HkTelemetryPkt.SciPassCount);
            }
            break;

        /*
        ** Reset Science Pass Counter
        */
        case MGR_SCI_PASS_RESET_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                MGR_AppData.HkTelemetryPkt.SciPassCount = 0;
                CFE_EVS_SendEvent(MGR_CMD_RESET_SCIPASS_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "MGR: RESET science pass counter command received");
            }
            break;

        /*
        ** Set AK Config (Enabled/Disabled)
        */
        case MGR_SET_AK_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_U8_cmd_t)) == OS_SUCCESS)
            {
                MGR_U8_cmd_t *ak_cmd = (MGR_U8_cmd_t *)MGR_AppData.MsgPtr;

                if (ak_cmd->U8 <= 1)
                {
                    MGR_AppData.HkTelemetryPkt.AkConfig = ak_cmd->U8;
                    CFE_EVS_SendEvent(MGR_CMD_SETAK_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "MGR: En/Dis AK Region command received, value: [%d]",
                                      MGR_AppData.HkTelemetryPkt.AkConfig);
                }
                else
                {
                    CFE_EVS_SendEvent(MGR_CMD_SETAK_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "MGR: Invalid En/Dis AK Region command received [%d]", ak_cmd->U8);
                }
            }
            break;

        /*
        ** Set CONUS Config (Enabled/Disabled)
        */
        case MGR_SET_CONUS_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_U8_cmd_t)) == OS_SUCCESS)
            {
                MGR_U8_cmd_t *conus_cmd = (MGR_U8_cmd_t *)MGR_AppData.MsgPtr;

                if (conus_cmd->U8 <= 1)
                {
                    MGR_AppData.HkTelemetryPkt.ConusConfig = conus_cmd->U8;
                    CFE_EVS_SendEvent(MGR_CMD_SETCONUS_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "MGR: En/Dis CONUS Region command received, value: [%d]",
                                      MGR_AppData.HkTelemetryPkt.ConusConfig);
                }
                else
                {
                    CFE_EVS_SendEvent(MGR_CMD_SETCONUS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "MGR: Invalid En/Dis CONUS Region command received [%d]", conus_cmd->U8);
                }
            }

            break;

        /*
        ** Set HI Config (Enabled/Disabled)
        */
        case MGR_SET_HI_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_U8_cmd_t)) == OS_SUCCESS)
            {
                MGR_U8_cmd_t *hi_cmd = (MGR_U8_cmd_t *)MGR_AppData.MsgPtr;

                if (hi_cmd->U8 <= 1)
                {
                    MGR_AppData.HkTelemetryPkt.HiConfig = hi_cmd->U8;
                    CFE_EVS_SendEvent(MGR_CMD_SETHI_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "MGR: En/Dis HI Region command received, value: [%d]",
                                      MGR_AppData.HkTelemetryPkt.HiConfig);
                }
                else
                {
                    CFE_EVS_SendEvent(MGR_CMD_SETHI_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "MGR: Invalid En/Dis HI Region command received [%d]", hi_cmd->U8);
                }
            }
            break;

        /*
        ** Update Science Status Command
        ** NOTE: NOT sent from the ground, sent by RTSs for telemetry usage by operators
        ** Indicates what science is/has happened
        */
        case MGR_UPDATE_SCI_STATUS_CC:
            if (MGR_VerifyCmdLength(MGR_AppData.MsgPtr, sizeof(MGR_U8_cmd_t)) == OS_SUCCESS)
            {
                MGR_U8_cmd_t *status_cmd = (MGR_U8_cmd_t *)MGR_AppData.MsgPtr;
                if (status_cmd->U8 > SS_EXITED_SCIENCE_MODE)
                {
                    CFE_EVS_SendEvent(MGR_CMD_UPDATE_SCI_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "MGR: Invalid science status update, received [%d], status remains [%d]",
                                      status_cmd->U8, MGR_AppData.HkTelemetryPkt.ScienceStatus);
                }
                else
                {
                    MGR_AppData.HkTelemetryPkt.ScienceStatus = status_cmd->U8;
                    CFE_EVS_SendEvent(MGR_CMD_UPDATE_SCI_STATUS_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "MGR: Set science status command received [%d]",
                                      MGR_AppData.HkTelemetryPkt.ScienceStatus);
                }
            }
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            MGR_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(MGR_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MGR: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x",
                              CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
}

/*
** Process Telemetry Request - Triggered in response to a telemetery request
*/
void MGR_ProcessTelemetryRequest(void)
{
    CFE_SB_MsgId_t    MsgId       = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /* MsgId is only needed if the command code is not recognized. See default case */
    CFE_MSG_GetMsgId(MGR_AppData.MsgPtr, &MsgId);

    /* Pull this command code from the message and then process */
    CFE_MSG_GetFcnCode(MGR_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        case MGR_REQ_HK_TLM:
            MGR_ReportHousekeeping();
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            MGR_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(MGR_DEVICE_TLM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MGR: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x",
                              CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
}

/*
** Report Application Housekeeping
*/
void MGR_ReportHousekeeping(void)
{
    OS_time_t temp_time;

    /* Update stored time */
    OS_GetLocalTime(&temp_time);
    MGR_AppData.HkTelemetryPkt.TimeTics = temp_time.ticks;

    /* Time stamp and publish housekeeping telemetry */
    CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&MGR_AppData.HkTelemetryPkt);
    CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&MGR_AppData.HkTelemetryPkt, true);

    /* Save Hk file*/
    MGR_SaveHkFile();
    OS_printf("MGR: TimeTics = %ld \n", MGR_AppData.HkTelemetryPkt.TimeTics);

    return;
}

/*
** Reset all global counter variables
*/
void MGR_ResetCounters(void)
{
    MGR_AppData.HkTelemetryPkt.CommandErrorCount = 0;
    MGR_AppData.HkTelemetryPkt.CommandCount      = 0;
    return;
}

/*
** Save the MGR Hk data to a file
*/
void MGR_SaveHkFile(void)
{
    int32     status = OS_SUCCESS;
    osal_id_t osal_fd;

    status = OS_OpenCreate(&osal_fd, MGR_CFG_FILE_PATH, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE, OS_READ_WRITE);
    if (status != OS_SUCCESS)
    {
        OS_printf("MGR Error: Cannot Open %s for writing. Error = %d \n", MGR_CFG_FILE_PATH, (int32)osal_fd);
    }
    else
    {
        status = OS_write(osal_fd, &MGR_AppData.HkTelemetryPkt, sizeof(MGR_Hk_tlm_t));
        if (status != sizeof(MGR_Hk_tlm_t))
        {
            OS_printf("MGR Error: Hk packet not written to file\n");
        }
        OS_close(osal_fd);
    }
}

/*
** Restore the MGR HK data from a file
*/
void MGR_RestoreHkFile(void)
{
    int32     status = OS_SUCCESS;
    osal_id_t osal_fd;
    int32     size_read;
    OS_time_t temp_time;

    status = OS_OpenCreate(&osal_fd, MGR_CFG_FILE_PATH, OS_FILE_FLAG_NONE, OS_READ_ONLY);
    if (status != OS_SUCCESS)
    {
        OS_printf("MGR: Restore Hk packet Error: Cannot Open %s for reading\n", MGR_CFG_FILE_PATH);
        OS_printf("      Cleaning up and recreating file\n");
        /* Clean up and re-create the file */
        OS_remove(MGR_CFG_FILE_PATH);
        OS_OpenCreate(&osal_fd, MGR_CFG_FILE_PATH, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE, OS_READ_WRITE);
        OS_write(osal_fd, &MGR_AppData.HkTelemetryPkt, sizeof(MGR_Hk_tlm_t));

        /* Initialize packet */
        CFE_MSG_Init(CFE_MSG_PTR(MGR_AppData.HkTelemetryPkt.TlmHeader), CFE_SB_ValueToMsgId(MGR_HK_TLM_MID),
                     MGR_HK_TLM_LNGTH);
    }
    else
    {
        size_read = OS_TimedRead(osal_fd, &MGR_AppData.HkTelemetryPkt, sizeof(MGR_Hk_tlm_t), 500);
        if (size_read != sizeof(MGR_Hk_tlm_t))
        {
            OS_printf("MGR: Restore Hk packet error: OS_read returned error\n");
            memset(&MGR_AppData.HkTelemetryPkt, 0, sizeof(MGR_Hk_tlm_t));

            /* Initialize packet */
            CFE_MSG_Init(CFE_MSG_PTR(MGR_AppData.HkTelemetryPkt.TlmHeader), CFE_SB_ValueToMsgId(MGR_HK_TLM_MID),
                         MGR_HK_TLM_LNGTH);
        }
    }
    OS_close(osal_fd);

    /*
    ** Make sure the SpacecraftMode is valid and set boot counters
    **/
    if ((MGR_AppData.HkTelemetryPkt.SpacecraftMode != MGR_SAFE_REBOOT_MODE) &&
        (MGR_AppData.HkTelemetryPkt.SpacecraftMode != MGR_SCIENCE_REBOOT_MODE))
    {
        MGR_AppData.HkTelemetryPkt.SpacecraftMode = MGR_SAFE_REBOOT_MODE;
        MGR_AppData.HkTelemetryPkt.AnomRebootCtr++;
        OS_printf("MGR: Restore Hk Packet error: Anomalous reboot, returning to SAFE MODE\n");
    }
    else
    {
        MGR_AppData.HkTelemetryPkt.BootCounter++;
    }

    /*
     * Set the time with the boot offset
     */
    OS_GetLocalTime(&temp_time);
    temp_time.ticks = temp_time.ticks + MGR_CFG_REBOOT_TIME_TIC_OFFSET + MGR_AppData.HkTelemetryPkt.TimeTics;
    OS_SetLocalTime(&temp_time);
    OS_printf("MGR: Calling OS_SetLocalTime with ticks: %ld\n", MGR_AppData.HkTelemetryPkt.TimeTics);
}

/*
** Verify command packet length matches expected
*/
int32 MGR_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length)
{
    int32             status        = OS_SUCCESS;
    CFE_SB_MsgId_t    msg_id        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t cmd_code      = 0;
    size_t            actual_length = 0;

    CFE_MSG_GetSize(msg, &actual_length);
    if (expected_length == actual_length)
    {
        /* Increment the command counter upon receipt of an invalid command */
        MGR_AppData.HkTelemetryPkt.CommandCount++;
    }
    else
    {
        CFE_MSG_GetMsgId(msg, &msg_id);
        CFE_MSG_GetFcnCode(msg, &cmd_code);

        CFE_EVS_SendEvent(MGR_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid msg length: ID = 0x%X,  CC = %d, Len = %ld, Expected = %d",
                          CFE_SB_MsgIdToValue(msg_id), cmd_code, actual_length, expected_length);

        status = OS_ERROR;

        /* Increment the command error counter upon receipt of an invalid command */
        MGR_AppData.HkTelemetryPkt.CommandErrorCount++;
    }
    return status;
}
