/************************************************************************
** File:
**   $Id: raspberry_pi_msgids.h  $
**
** Purpose:
**  Define RASPBERRY_PI Message IDs
**
*************************************************************************/
#ifndef _RASPBERRY_PI_MSGIDS_H_
#define _RASPBERRY_PI_MSGIDS_H_

/*
** CCSDS V1 Command Message IDs (MID) must be 0x18xx
*/
#define RASPBERRY_PI_CMD_MID 0x18FA /* TODO: Change this for your app */

/*
** This MID is for commands telling the app to publish its telemetry message
*/
#define RASPBERRY_PI_REQ_HK_MID 0x18FB /* TODO: Change this for your app */

/*
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define RASPBERRY_PI_HK_TLM_MID     0x08FA /* TODO: Change this for your app */
#define RASPBERRY_PI_DEVICE_TLM_MID 0x08FB /* TODO: Change this for your app */

#endif /* _RASPBERRY_PI_MSGIDS_H_ */
