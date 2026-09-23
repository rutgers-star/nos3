/************************************************************************
** File:
**   $Id: payload_if_platform_cfg.h  $
**
** Purpose:
**  Define payload_if Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _PAYLOAD_IF_PLATFORM_CFG_H_
#define _PAYLOAD_IF_PLATFORM_CFG_H_

/*
** Default PAYLOAD_IF Configuration
*/
#ifndef PAYLOAD_IF_CFG
/* Notes:
**   NOS3 uart requires matching handle and bus number
*/
#define PAYLOAD_IF_CFG_STRING      "usart_16"
#define PAYLOAD_IF_CFG_HANDLE      16
#define PAYLOAD_IF_CFG_BAUDRATE_HZ 115200
#define PAYLOAD_IF_CFG_MS_TIMEOUT  50 /* Max 255 */
/* Note: Debug flag disabled (commented out) by default */
//#define PAYLOAD_IF_CFG_DEBUG
#endif

#endif /* _PAYLOAD_IF_PLATFORM_CFG_H_ */
