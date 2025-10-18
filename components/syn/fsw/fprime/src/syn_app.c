/*******************************************************************************
** File: syn_app.c
**
** Purpose:
**   This file contains the source code for the SYN application.
**
*******************************************************************************/

/*
** Include Files
*/
#include <arpa/inet.h>
#include "syn_app.h"
#include "itc_synopsis_bridge.h"

/*
** Global Data
*/
SYN_AppData_t SYN_AppData;

void* memory;
size_t mem_req_bytes;
double sigma = 1.34289567767;
float dl_size = 5.0;
int num_files_downlinked = 0;


/*
** Application entry point and main process loop
*/
void SYN_AppMain(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SYN_PERF_ID);

    /* 
    ** Perform application initialization
    */
    status = SYN_AppInit();
    if (status != CFE_SUCCESS)
    {
        SYN_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Main loop
    */
    while (CFE_ES_RunLoop(&SYN_AppData.RunStatus) == true)
    {
        /*
        ** Performance log exit stamp
        */
        CFE_ES_PerfLogExit(SYN_PERF_ID);

        /* 
        ** Pend on the arrival of the next Software Bus message
        ** Note that this is the standard, but timeouts are available
        */
        status = CFE_SB_ReceiveBuffer((CFE_SB_Buffer_t **)&SYN_AppData.MsgPtr,  SYN_AppData.CmdPipe,  CFE_SB_PEND_FOREVER);
        
        /* 
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution.
        */
        CFE_ES_PerfLogEntry(SYN_PERF_ID);

        /*
        ** If the CFE_SB_ReceiveBuffer was successful, then continue to process the command packet
        ** If not, then exit the application in error.
        ** Note that a SB read error should not always result in an app quitting.
        */
        if (status == CFE_SUCCESS)
        {
            SYN_ProcessCommandPacket();
        }
        else
        {
            CFE_EVS_SendEvent(SYN_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SYN: SB Pipe Read Error = %d", (int) status);
            SYN_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Disable component, which cleans up the interface, upon exit
    */
    //SYN_Disable();

    /*
    ** Performance log exit stamp
    */
    CFE_ES_PerfLogExit(SYN_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(SYN_AppData.RunStatus);
} 


/* 
** Initialize application
*/
int32 SYN_AppInit(void)
{
    int32 status = OS_SUCCESS;
    
    SYN_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */ 
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);    /* as default, no filters are used */
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SYN: Error registering for event services: 0x%08X\n", (unsigned int) status);
       return status;
    }

    /*
    ** Create the Software Bus command pipe 
    */
    status = CFE_SB_CreatePipe(&SYN_AppData.CmdPipe, SYN_PIPE_DEPTH, "SYN_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SYN_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
            "Error Creating SB Pipe,RC=0x%08X",(unsigned int) status);
       return status;
    }
    
    /*
    ** Subscribe to ground commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SYN_CMD_MID), SYN_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SYN_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
            "Error Subscribing to HK Gnd Cmds, MID=0x%04X, RC=0x%08X",
            SYN_CMD_MID, (unsigned int) status);
        return status;
    }

    /*
    ** Subscribe to housekeeping (hk) message requests
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SYN_REQ_HK_MID), SYN_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SYN_SUB_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
            "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X",
            SYN_REQ_HK_MID, (unsigned int) status);
        return status;
    }


    /* 
    ** Initialize the published HK message - this HK message will contain the 
    ** telemetry that has been defined in the SYN_HkTelemetryPkt for this app.
    */
    CFE_MSG_Init(CFE_MSG_PTR(SYN_AppData.HkTelemetryPkt.TlmHeader),
                   CFE_SB_ValueToMsgId(SYN_HK_TLM_MID),
                   SYN_HK_TLM_LNGTH);

    /*
    ** Initialize the device packet message
    ** This packet is specific to your application
    */
    // CFE_MSG_Init(CFE_MSG_PTR(SYN_AppData.DevicePkt.TlmHeader),
    //                CFE_SB_ValueToMsgId(SYN_DEVICE_TLM_MID),
    //                SYN_DEVICE_TLM_LNGTH);


    /*
    ** ITC SYNOPSIS Ex Setup
    */  

    // Determine memory requirements
    mem_req_bytes = 0;
    mem_req_bytes = itc_app_get_memory_requiremennt();

    
    // Set up Passthrough ASDS
    itc_setup_ptasds();

    // Initalize App
    memory = malloc(mem_req_bytes);  // Malloc for this example.  
    itc_app_init(mem_req_bytes, memory);    
    

    /* 
    ** Always reset all counters during application initialization 
    */
    SYN_ResetCounters();

    /* 
     ** Send an information event that the app has initialized. 
     ** This is useful for debugging the loading of individual applications.
     */
    status = CFE_EVS_SendEvent(SYN_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
               "SYN App Initialized. Version %d.%d.%d.%d",
                SYN_MAJOR_VERSION,
                SYN_MINOR_VERSION, 
                SYN_REVISION, 
                SYN_MISSION_REV);	
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SYN: Error sending initialization event: 0x%08X\n", (unsigned int) status);
    }
    return status;
} 


/* 
** Process packets received on the SYN command pipe
*/
void SYN_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(SYN_AppData.MsgPtr, &MsgId);
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        /*
        ** Ground Commands with command codes fall under the SYN_CMD_MID (Message ID)
        */
        case SYN_CMD_MID:
            SYN_ProcessGroundCommand();
            break;

        /*
        ** All other messages, other than ground commands, add to this case statement.
        */
        case SYN_REQ_HK_MID:
            SYN_ProcessTelemetryRequest();
            break;

        /*
        ** All other invalid messages that this app doesn't recognize, 
        ** increment the command error counter and log as an error event.  
        */
        default:
            SYN_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SYN_PROCESS_CMD_ERR_EID,CFE_EVS_EventType_ERROR, "SYN: Invalid command packet, MID = 0x%x", CFE_SB_MsgIdToValue(MsgId));
            break;
    }
    return;
} 


/*
** Process ground commands
*/
void SYN_ProcessGroundCommand(void)
{
    int32 status = OS_SUCCESS;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /*
    ** MsgId is only needed if the command code is not recognized. See default case
    */
    CFE_MSG_GetMsgId(SYN_AppData.MsgPtr, &MsgId);

    /*
    ** Ground Commands, by definition, have a command code (_CC) associated with them
    ** Pull this command code from the message and then process
    */
    CFE_MSG_GetFcnCode(SYN_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case SYN_NOOP_CC:
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SYN_CMD_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SYN: NOOP command received");
            }
            break;

        /*
        ** Reset Counters Command
        */
        case SYN_RESET_COUNTERS_CC:
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SYN_CMD_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SYN: RESET counters command received");
                SYN_ResetCounters();
            }
            break;

        /*
        ** Add Data Command
        */
        // Add data to the synopsis database.  
        // Users must know where the experiment data lives.
        // This is currently handed as a simple example within the synopsis_bridge file.  Data paths can be handled in either location.  Here or within the bridge.
        case SYN_ADD_DATA_CC:
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SYN_CMD_ADD_DATA_EID, CFE_EVS_EventType_INFORMATION, "SYN: Config ADD DATA command received");

                int syn_app_status;

                syn_app_status = owls_add_dpmsg(); // No Data passed in.  Datapath within itc_synopsis_bridge.cpp

                if(syn_app_status == 0) //Success
                {         
                    OS_printf("SYN: DATA PRODUCT ADDED -> SYNOPSIS DB\n");
                }
                else
                {
                    OS_printf("*! SYN:  Unable to add additional DPMSG!\n");
                }
            }        
            break; 

        /*  
        ** Configure Downlink Command
        */
        // Synopsis can be configured to prioritize based on size constraints.  This can be configured here.
        case SYN_CONFIG_DL_CC:
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_Config_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SYN_CMD_CONFIG_DL_EID, CFE_EVS_EventType_INFORMATION, "SYN: Config DL command received");

                dl_size = ((((SYN_Config_cmd_t*) SYN_AppData.MsgPtr)->DeviceCfg) / 10.0);
                OS_printf("** SYNOPSIS DL SIZE SET: %f\n", dl_size);
            }
            break;
            
        /*
        ** Configure Sigma Command
        */
        // The Alpha value can be utilized to adjust the broadness of desired data
        case SYN_CONFIG_ALPHA_CC:
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_Config_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SYN_CMD_CONFIG_ALPHA_EID, CFE_EVS_EventType_INFORMATION, "SYN: Config ALPHA command received");
                
                if (status == OS_SUCCESS)
                {
                    sigma = ((((SYN_Config_cmd_t*) SYN_AppData.MsgPtr)->DeviceCfg) / 100.0);
                    
                    owls_set_sigma(sigma);
                    OS_printf("** SYNOPSIS ALPHA SET: %f\n", sigma);
                }
            }
            break;
        /*
        ** Reset SYNOPSIS Command
        */
        case SYN_RESET_CC: 
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                int32 reset_status = OS_ERROR;
                OS_printf("* SYNOPSIS RESET IN PROGRESS!\n");
                //SYN_APP_ResetCounters();
                dl_size = 5.0;
                num_files_downlinked = 0;

                CFE_EVS_SendEvent(SYN_CMD_RESET_DEMO_EID, CFE_EVS_EventType_INFORMATION, "SYN: RESET command received");
   
                itc_app_deinit(memory); 
                memory = malloc(mem_req_bytes);
               
                reset_dp_counter();
                
                reset_status = OS_remove(SYN_DATABASE_PATH);
                if(reset_status != OS_SUCCESS)
                {
                    OS_printf("Error Removing file: %s\nError: %d", SYN_DATABASE_PATH, reset_status);
                }
                else
                {
                    reset_status = OS_cp(SYN_DATABASE_CLEAN_PATH, SYN_DATABASE_PATH);
                    if(reset_status != OS_SUCCESS)
                    {
                        OS_printf("Error Resetting Database File to: %s\n", SYN_DATABASE_CLEAN_PATH);
                    }
                    else
                    {
                        itc_app_init(mem_req_bytes, memory);
                    }
                } 
            }
            break;
        /*
        ** Prioritize Data Command
        */
        case SYN_PRIO_CC:
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SYN_CMD_PRIO_EID, CFE_EVS_EventType_INFORMATION, "SYN: PRIORITIZE command received");

                owls_prioritize_data();
                
                OS_printf("** PRIORITIZATION COMPLETE\n");
            }
            break;

        /*
        ** Get Prioritized Data Command
        */
        // Synopsis wants to be part of the downlink process.
        // It doesn't have to handle it, but it needs to know which files under analysis have been downlinked.
        // This allows synopsis to no longer include it in its prioritization analysis.

        // Currently this command code will remove the top priority file, as an example, from the synopsis db, and then re-prioritize
        case SYN_GET_PDATA_CC:
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SYN_CMD_GET_PDATA_EID, CFE_EVS_EventType_INFORMATION, "SYN: GET_PDATA command received");
                
                owls_update_downlink_status_prio1();
                
                OS_printf("** SYN: Updated  DL Status Complete\n");
            }
            break;

        /*
        ** Display Prioritized Data Command
        */
        case SYN_DISP_PDATA_CC:
            if (SYN_VerifyCmdLength(SYN_AppData.MsgPtr, sizeof(SYN_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SYN_CMD_DISP_PDATA_EID, CFE_EVS_EventType_INFORMATION, "SYN: DISPLAY PDATA command received");
                owls_display_prioritized_data();  
            }
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            SYN_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SYN_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                "SYN: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
} 


/*
** Process Telemetry Request - Triggered in response to a telemetery request
*/
void SYN_ProcessTelemetryRequest(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /* MsgId is only needed if the command code is not recognized. See default case */
    CFE_MSG_GetMsgId(SYN_AppData.MsgPtr, &MsgId);

    /* Pull this command code from the message and then process */
    CFE_MSG_GetFcnCode(SYN_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        case SYN_REQ_HK_TLM:
            SYN_ReportHousekeeping();
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            SYN_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SYN_DEVICE_TLM_ERR_EID, CFE_EVS_EventType_ERROR, 
                "SYN: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
}


/* 
** Report Application Housekeeping
*/
void SYN_ReportHousekeeping(void)
{
    int32 status = OS_SUCCESS;
    /* Time stamp and publish housekeeping telemetry */
    CFE_SB_TimeStampMsg((CFE_MSG_Message_t *) &SYN_AppData.HkTelemetryPkt);
    CFE_SB_TransmitMsg((CFE_MSG_Message_t *) &SYN_AppData.HkTelemetryPkt, true);
    return;
}



/*
** Reset all global counter variables
*/
void SYN_ResetCounters(void)
{
    SYN_AppData.HkTelemetryPkt.CommandErrorCount = 0;
    SYN_AppData.HkTelemetryPkt.CommandCount = 0;
    return;
} 

/*
** Verify command packet length matches expected
*/
int32 SYN_VerifyCmdLength(CFE_MSG_Message_t * msg, uint16 expected_length)
{     
    int32 status = OS_SUCCESS;
    CFE_SB_MsgId_t msg_id = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t cmd_code = 0;
    size_t actual_length = 0;

    CFE_MSG_GetSize(msg, &actual_length);
    if (expected_length == actual_length)
    {
        /* Increment the command counter upon receipt of an invalid command */
        SYN_AppData.HkTelemetryPkt.CommandCount++;
    }
    else
    {
        CFE_MSG_GetMsgId(msg, &msg_id);
        CFE_MSG_GetFcnCode(msg, &cmd_code);

        CFE_EVS_SendEvent(SYN_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              CFE_SB_MsgIdToValue(msg_id), cmd_code, actual_length, expected_length);

        status = OS_ERROR;

        /* Increment the command error counter upon receipt of an invalid command */
        SYN_AppData.HkTelemetryPkt.CommandErrorCount++;
    }
    return status;
} 


