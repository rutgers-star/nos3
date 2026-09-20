/*******************************************************************************
** File: tmp100_app.h
**
** Purpose:
**   This is the main header file for the TMP100 application.
**
*******************************************************************************/
#ifndef _TMP100_APP_H_
#define _TMP100_APP_H_

/*
** Include Files
*/
#include "cfe.h"
#include "tmp100_device.h"
#include "tmp100_events.h"
#include "tmp100_platform_cfg.h"
#include "tmp100_perfids.h"
#include "tmp100_msg.h"
#include "tmp100_msgids.h"
#include "tmp100_version.h"
#include "hwlib.h"

/* TODO: This is specific to the tmp100 application, remove if using template generator */
#include "mgr_msg.h"
#include "mgr_msgids.h"

/*
** Specified pipe depth - how many messages will be queued in the pipe
*/
#define TMP100_PIPE_DEPTH 32

/*
** Enabled and Disabled Definitions
*/
#define TMP100_DEVICE_DISABLED 0
#define TMP100_DEVICE_ENABLED  1

/*
** TMP100 global data structure
** The cFE convention is to put all global app data in a single struct.
** This struct is defined in the `tmp100_app.h` file with one global instance
** in the `.c` file.
*/
typedef struct
{
    /*
    ** Housekeeping telemetry packet
    ** Each app defines its own packet which contains its OWN telemetry
    */
    TMP100_Hk_tlm_t HkTelemetryPkt; /* TMP100 Housekeeping Telemetry Packet */

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
    TMP100_Device_tlm_t DevicePkt; /* Device specific data packet */

    /*
    ** Device protocol
    ** TMP100 uses I2C communication
    */
    i2c_bus_info_t Tmp100I2C; /* I2C bus info for TMP100 device */

} TMP100_AppData_t;

/*
** Exported Data
** Extern the global struct in the header for the Unit Test Framework (UTF).
*/
extern TMP100_AppData_t TMP100_AppData; /* TMP100 App Data */

/*
**
** Local function prototypes.
**
** Note: Except for the entry point (TMP100_AppMain), these
**       functions are not called from any other source module.
*/
void  TMP100_AppMain(void);
int32 TMP100_AppInit(void);
void  TMP100_ProcessCommandPacket(void);
void  TMP100_ProcessGroundCommand(void);
void  TMP100_ProcessTelemetryRequest(void);
void  TMP100_ReportHousekeeping(void);
void  TMP100_ReportDeviceTelemetry(void);
void  TMP100_ResetCounters(void);
void  TMP100_Enable(void);
void  TMP100_Disable(void);
void  TMP100_Configure(void);
int32 TMP100_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length);

/* TODO: This is specific to the tmp100 application, remove if using template generator */
void TMP100_ProcessMgrHk(void);

#endif /* _TMP100_APP_H_ */
