/************************************************************************
** File:
**   $Id: tmp100_msgids.h  $
**
** Purpose:
**  Define TMP100 Message IDs
**
*************************************************************************/
#ifndef _TMP100_MSGIDS_H_
#define _TMP100_MSGIDS_H_

/*
** CCSDS V1 Command Message IDs (MID) must be 0x18xx
*/
#define TMP100_CMD_MID 0x1A00

/*
** This MID is for commands telling the app to publish its telemetry message
*/
#define TMP100_REQ_HK_MID 0x1A01

/*
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define TMP100_HK_TLM_MID     0x0A00
#define TMP100_DEVICE_TLM_MID 0x0A01

#endif /* _TMP100_MSGIDS_H_ */
