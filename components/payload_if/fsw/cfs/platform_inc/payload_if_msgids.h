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
** 0x1860 and above is the cFE global command range; 0x1860 is CFE_TIME_DATA_CMD_MID
*/
#define PAYLOAD_IF_CMD_MID 0x1850

/*
** This MID is for commands telling the app to publish its telemetry message
*/
#define PAYLOAD_IF_REQ_HK_MID 0x1851

/*
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define PAYLOAD_IF_HK_TLM_MID     0x0860
#define PAYLOAD_IF_DEVICE_TLM_MID 0x0861

/*
** PayOBC telemetry that PAYLOAD_IF publishes unchanged from the payload link:
** APID 0x011 as a telemetry packet without a secondary header (ICD RevB D6).
** Kept numeric for the TO/TO_LAB tables; payload_if_app.c checks it against
** STAR_APID_PAYLOAD_TELEMETRY from payload-apids.
*/
#define PAYLOAD_IF_PAYOBC_TLM_MID 0x0011

#endif /* _PAYLOAD_IF_MSGIDS_H_ */
