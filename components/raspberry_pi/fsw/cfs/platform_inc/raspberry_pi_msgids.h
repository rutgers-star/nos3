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
#define RASPBERRY_PI_CMD_MID 0x1900

/*
** This MID is for commands telling the app to publish its telemetry message
*/
#define RASPBERRY_PI_REQ_HK_MID 0x1901

/*
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define RASPBERRY_PI_HK_TLM_MID     0x0900
#define RASPBERRY_PI_DEVICE_TLM_MID 0x0901

/*
** Data Message IDs for SBN communication
*/
#define RASPBERRY_PI_DATA_IN_MID    0x0902  /* Data from Pi via SBN */
#define RASPBERRY_PI_DATA_OUT_MID   0x1902  /* Data to Pi via SBN */

#endif /* _RASPBERRY_PI_MSGIDS_H_ */
