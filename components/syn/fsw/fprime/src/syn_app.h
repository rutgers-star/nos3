/*******************************************************************************
** File: syn_app.h
**
** Purpose:
**   This is the main header file for the SYN application.
**
*******************************************************************************/
#ifndef _SYN_APP_H_
#define _SYN_APP_H_

/*
** Include Files
*/
#include "cfe.h"
#include "syn_events.h"
#include "syn_platform_cfg.h"
#include "syn_perfids.h"
#include "syn_msg.h"
#include "syn_msgids.h"
#include "syn_version.h"
//#include "hwlib.h"


/*
** Specified pipe depth - how many messages will be queued in the pipe
*/
#define SYN_PIPE_DEPTH            32


/*
** Enabled and Disabled Definitions
*/
#define SYN_DEVICE_DISABLED       0
#define SYN_DEVICE_ENABLED        1


/*
** SYN global data structure
** The cFE convention is to put all global app data in a single struct. 
** This struct is defined in the `syn_app.h` file with one global instance 
** in the `.c` file.
*/
typedef struct
{
    /*
    ** Housekeeping telemetry packet
    ** Each app defines its own packet which contains its OWN telemetry
    */
    SYN_Hk_tlm_t   HkTelemetryPkt;   /* SYN Housekeeping Telemetry Packet */
    
    /*
    ** Operational data  - not reported in housekeeping
    */
    CFE_MSG_Message_t * MsgPtr;             /* Pointer to msg received on software bus */
    CFE_SB_PipeId_t CmdPipe;            /* Pipe Id for HK command pipe */
    uint32 RunStatus;                   /* App run status for controlling the application state */
} SYN_AppData_t;


/*
** Exported Data
** Extern the global struct in the header for the Unit Test Framework (UTF).
*/
extern SYN_AppData_t SYN_AppData; /* SYN App Data */


/*
**
** Local function prototypes.
**
** Note: Except for the entry point (SYN_AppMain), these
**       functions are not called from any other source module.
*/
void  SYN_AppMain(void);
int32 SYN_AppInit(void);
void  SYN_ProcessCommandPacket(void);
void  SYN_ProcessGroundCommand(void);
void  SYN_ProcessTelemetryRequest(void);
void  SYN_ReportHousekeeping(void);
void  SYN_ReportDeviceTelemetry(void);
void  SYN_ResetCounters(void);
int32 SYN_VerifyCmdLength(CFE_MSG_Message_t * msg, uint16 expected_length);

#endif /* _SYN_APP_H_ */
