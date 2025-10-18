/*******************************************************************************
** File: raspberry_pi_app.h
**
** Purpose:
**   This is the main header file for the RASPBERRY_PI application.
**
*******************************************************************************/
#ifndef _RASPBERRY_PI_APP_H_
#define _RASPBERRY_PI_APP_H_

/*
** Include Files
*/
#include "cfe.h"
#include "raspberry_pi_device.h"
#include "raspberry_pi_events.h"
#include "raspberry_pi_platform_cfg.h"
#include "raspberry_pi_perfids.h"
#include "raspberry_pi_msg.h"
#include "raspberry_pi_msgids.h"
#include "raspberry_pi_version.h"
#include "hwlib.h"

/* TODO: This is specific to the raspberry_pi application, remove if using template generator */
#include "mgr_msg.h"
#include "mgr_msgids.h"

/*
** Specified pipe depth - how many messages will be queued in the pipe
*/
#define RASPBERRY_PI_PIPE_DEPTH 32

/*
** Enabled and Disabled Definitions
*/
#define RASPBERRY_PI_DEVICE_DISABLED 0
#define RASPBERRY_PI_DEVICE_ENABLED  1

/*
** RASPBERRY_PI global data structure
** The cFE convention is to put all global app data in a single struct.
** This struct is defined in the `raspberry_pi_app.h` file with one global instance
** in the `.c` file.
*/
typedef struct
{
    /*
    ** Housekeeping telemetry packet
    ** Each app defines its own packet which contains its OWN telemetry
    */
    RASPBERRY_PI_Hk_tlm_t HkTelemetryPkt; /* RASPBERRY_PI Housekeeping Telemetry Packet */

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
    RASPBERRY_PI_Device_tlm_t DevicePkt; /* Device specific data packet */

    /*
    ** Device protocol
    ** TODO: Make specific to your application
    */
    uart_info_t Raspberry_piUart; /* Hardware protocol definition */

} RASPBERRY_PI_AppData_t;

/*
** Exported Data
** Extern the global struct in the header for the Unit Test Framework (UTF).
*/
extern RASPBERRY_PI_AppData_t RASPBERRY_PI_AppData; /* RASPBERRY_PI App Data */

/*
**
** Local function prototypes.
**
** Note: Except for the entry point (RASPBERRY_PI_AppMain), these
**       functions are not called from any other source module.
*/
void  RASPBERRY_PI_AppMain(void);
int32 RASPBERRY_PI_AppInit(void);
void  RASPBERRY_PI_ProcessCommandPacket(void);
void  RASPBERRY_PI_ProcessGroundCommand(void);
void  RASPBERRY_PI_ProcessTelemetryRequest(void);
void  RASPBERRY_PI_ReportHousekeeping(void);
void  RASPBERRY_PI_ReportDeviceTelemetry(void);
void  RASPBERRY_PI_ResetCounters(void);
void  RASPBERRY_PI_Enable(void);
void  RASPBERRY_PI_Disable(void);
void  RASPBERRY_PI_Configure(void);
int32 RASPBERRY_PI_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length);

/* TODO: This is specific to the raspberry_pi application, remove if using template generator */
void RASPBERRY_PI_ProcessMgrHk(void);

#endif /* _RASPBERRY_PI_APP_H_ */
