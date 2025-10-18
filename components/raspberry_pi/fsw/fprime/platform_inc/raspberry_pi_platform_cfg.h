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
*/
#define RASPBERRY_PI_CFG_STRING      "usart_16"
#define RASPBERRY_PI_CFG_HANDLE      16
#define RASPBERRY_PI_CFG_BAUDRATE_HZ 115200
#define RASPBERRY_PI_CFG_MS_TIMEOUT  50 /* Max 255 */
/* Note: Debug flag disabled (commented out) by default */
//#define RASPBERRY_PI_CFG_DEBUG
#endif

#endif /* _RASPBERRY_PI_PLATFORM_CFG_H_ */
