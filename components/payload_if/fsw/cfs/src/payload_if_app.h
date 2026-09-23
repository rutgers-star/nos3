/*******************************************************************************
** File: payload_if_app.h
**
** Purpose:
**   This is the main header file for the PAYLOAD_IF application.
**
*******************************************************************************/
#ifndef _PAYLOAD_IF_APP_H_
#define _PAYLOAD_IF_APP_H_

/*
** Include Files
*/
#include "cfe.h"
#include "payload_if_device.h"
#include "payload_if_events.h"
#include "payload_if_platform_cfg.h"
#include "payload_if_perfids.h"
#include "payload_if_msg.h"
#include "payload_if_msgids.h"
#include "payload_if_version.h"
#include "hwlib.h"
#include "payload_link/frame.h"
#include "star/payload_apids.h"

/* TODO: This is specific to the payload_if application, remove if using template generator */
#include "mgr_msg.h"
#include "mgr_msgids.h"

/*
** Specified pipe depth - how many messages will be queued in the pipe
*/
#define PAYLOAD_IF_PIPE_DEPTH 32

/*
** Enabled and Disabled Definitions
*/
#define PAYLOAD_IF_DEVICE_DISABLED 0
#define PAYLOAD_IF_DEVICE_ENABLED  1

/*
** PAYLOAD_IF global data structure
** The cFE convention is to put all global app data in a single struct.
** This struct is defined in the `payload_if_app.h` file with one global instance
** in the `.c` file.
*/
typedef struct
{
    /*
    ** Housekeeping telemetry packet
    ** Each app defines its own packet which contains its OWN telemetry
    */
    PAYLOAD_IF_Hk_tlm_t HkTelemetryPkt; /* PAYLOAD_IF Housekeeping Telemetry Packet */

    /*
    ** Operational data  - not reported in housekeeping
    */
    CFE_MSG_Message_t *MsgPtr;    /* Pointer to msg received on software bus */
    CFE_SB_PipeId_t    CmdPipe;   /* Pipe Id for HK command pipe */
    uint32             RunStatus; /* App run status for controlling the application state */

    /*
     ** Device data
     ** TODO: Make specific to your application
     */
    PAYLOAD_IF_Device_tlm_t DevicePkt; /* Device specific data packet */

    /*
    ** Device protocol
    ** TODO: Make specific to your application
    */
    uart_info_t Payload_ifUart; /* Hardware protocol definition */
    CFE_ES_TaskId_t          RxTaskID;      /* Child task ID for asynchronous UART receive */
    volatile bool            RxTaskRunning; /* Set true to run RxTask loop, false to stop it */
    plframe_decode_ctx_t     DecodeCtx;     /* Persistent payload-link decoder state */

} PAYLOAD_IF_AppData_t;

/*
** Exported Data
** Extern the global struct in the header for the Unit Test Framework (UTF).
*/
extern PAYLOAD_IF_AppData_t PAYLOAD_IF_AppData; /* PAYLOAD_IF App Data */

/*
**
** Local function prototypes.
**
** Note: Except for the entry point (PAYLOAD_IF_AppMain), these
**       functions are not called from any other source module.
*/
void  PAYLOAD_IF_AppMain(void);
int32 PAYLOAD_IF_AppInit(void);
void  PAYLOAD_IF_ProcessCommandPacket(void);
void  PAYLOAD_IF_ProcessGroundCommand(void);
void  PAYLOAD_IF_ProcessTelemetryRequest(void);
void  PAYLOAD_IF_ReportHousekeeping(void);
void  PAYLOAD_IF_ReportDeviceTelemetry(void);
void  PAYLOAD_IF_ResetCounters(void);
void  PAYLOAD_IF_Enable(void);
void  PAYLOAD_IF_Disable(void);
void  PAYLOAD_IF_Configure(void);
int32 PAYLOAD_IF_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length);
void  PAYLOAD_IF_RxTask(void);
void  PAYLOAD_IF_SendToPayload(void);
int32 PAYLOAD_IF_HandleDecodedFrame(void);

/* TODO: This is specific to the payload_if application, remove if using template generator */
void PAYLOAD_IF_ProcessMgrHk(void);

#endif /* _PAYLOAD_IF_APP_H_ */
