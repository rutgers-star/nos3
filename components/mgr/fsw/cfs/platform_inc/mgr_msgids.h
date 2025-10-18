/************************************************************************
** File:
**   $Id: mgr_msgids.h  $
**
** Purpose:
**  Define MGR Message IDs
**
*************************************************************************/
#ifndef _MGR_MSGIDS_H_
#define _MGR_MSGIDS_H_

/*
** CCSDS V1 Command Message IDs (MID)
*/
#define MGR_CMD_MID 0x18F8

/*
** This MID is for commands telling the app to publish its telemetry message
*/
#define MGR_REQ_HK_MID 0x18F9

/*
** CCSDS V1 Telemetry Message IDs
*/
#define MGR_HK_TLM_MID 0x08F8

#endif /* _MGR_MSGIDS_H_ */
