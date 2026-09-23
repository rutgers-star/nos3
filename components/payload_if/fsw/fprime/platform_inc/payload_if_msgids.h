/************************************************************************
** File:
**   $Id: payload_if_msgids.h  $
**
** Purpose:
**  Define PAYLOAD_IF Message IDs
**
*************************************************************************/
#ifndef _PAYLOAD_IF_MSGIDS_H_
#define _PAYLOAD_IF_MSGIDS_H_

/*
** CCSDS V1 Command Message IDs (MID) must be 0x18xx
*/
#define PAYLOAD_IF_CMD_MID 0x18FA /* TODO: Change this for your app */

/*
** This MID is for commands telling the app to publish its telemetry message
*/
#define PAYLOAD_IF_REQ_HK_MID 0x18FB /* TODO: Change this for your app */

/*
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define PAYLOAD_IF_HK_TLM_MID     0x08FA /* TODO: Change this for your app */
#define PAYLOAD_IF_DEVICE_TLM_MID 0x08FB /* TODO: Change this for your app */

#endif /* _PAYLOAD_IF_MSGIDS_H_ */
