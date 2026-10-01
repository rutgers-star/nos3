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
** cFE message ID of payload commands forwarded to the PayOBC. CCSDS v1 message
** IDs are the first 16 bits of the primary header, so the MID carries the
** packet type. ICD RevB D6 makes APID 0x010 a telecommand (type bit 0x1000)
** with no secondary header, which gives MID 0x1010.
*/
#define PAYLOAD_IF_CCSDS_TYPE_CMD 0x1000
#define PAYLOAD_IF_PAYOBC_CMD_MID (PAYLOAD_IF_CCSDS_TYPE_CMD | STAR_APID_PAYLOAD_COMMAND)

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
    PAYLOAD_IF_PayObc_tlm_t PayObcTlmPkt; /* PayOBC packet wrapped for the ground (RX task only) */

    /*
    ** Device protocol
    ** TODO: Make specific to your application
    */
    uart_info_t Payload_ifUart; /* Hardware protocol definition */
    CFE_ES_TaskId_t          RxTaskID;      /* Child task ID for asynchronous UART receive */
    volatile bool            RxTaskRunning; /* Set true to run RxTask loop, false to stop it */
    osal_id_t                RxTaskExitSem; /* Given by RxTask when it leaves its loop */
    uint16                   HkTickCount;   /* SCH ticks since the last periodic HK report */
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
void  PAYLOAD_IF_SetHkPeriod(void);
void  PAYLOAD_IF_ProcessHkTick(void);
int32 PAYLOAD_IF_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length);
void  PAYLOAD_IF_RxTask(void);
int32 PAYLOAD_IF_StartRxTask(void);
void  PAYLOAD_IF_StopRxTask(void);
void  PAYLOAD_IF_SendToPayload(void);
void  PAYLOAD_IF_SendPacketToPayload(const uint8 *Packet, size_t Length);
void  PAYLOAD_IF_ForwardToPayload(void);
int32 PAYLOAD_IF_HandleDecodedFrame(void);

/* TODO: This is specific to the payload_if application, remove if using template generator */
void PAYLOAD_IF_ProcessMgrHk(void);

#endif /* _PAYLOAD_IF_APP_H_ */
