/************************************************************************
** File:
**   $Id: raspberry_pi_platform_cfg.h  $
**
** Purpose:
**  Define raspberry_pi Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _RASPBERRY_PI_PLATFORM_CFG_H_
#define _RASPBERRY_PI_PLATFORM_CFG_H_

/*
** Default RASPBERRY_PI Configuration
*/
#ifndef RASPBERRY_PI_CFG
/* Notes:
**   NOS3 uart requires matching handle and bus number
**   Using UART 20 for Raspberry Pi component
*/
#define RASPBERRY_PI_CFG_STRING      "usart_20"
#define RASPBERRY_PI_CFG_HANDLE      20
#define RASPBERRY_PI_CFG_BAUDRATE_HZ 115200
#define RASPBERRY_PI_CFG_MS_TIMEOUT  50 /* Max 255 */

/* SBN Configuration for external Raspberry Pi */
#define RASPBERRY_PI_SBN_ENABLED     1
#define RASPBERRY_PI_PROCESSOR_ID    2
#define RASPBERRY_PI_SPACECRAFT_ID   0
#define RASPBERRY_PI_SBN_PORT        2234  /* Match SBN_CLIENT_PORT */

/* Note: Debug flag disabled (commented out) by default */
//#define RASPBERRY_PI_CFG_DEBUG
#endif

#endif /* _RASPBERRY_PI_PLATFORM_CFG_H_ */
