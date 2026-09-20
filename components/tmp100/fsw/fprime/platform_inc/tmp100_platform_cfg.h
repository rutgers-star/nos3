/************************************************************************
** File:
**   $Id: tmp100_platform_cfg.h  $
**
** Purpose:
**  Define tmp100 Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _TMP100_PLATFORM_CFG_H_
#define _TMP100_PLATFORM_CFG_H_

/*
** Default TMP100 Configuration
*/
#ifndef TMP100_CFG
/* Notes:
**   NOS3 uart requires matching handle and bus number
*/
#define TMP100_CFG_STRING      "usart_16"
#define TMP100_CFG_HANDLE      16
#define TMP100_CFG_BAUDRATE_HZ 115200
#define TMP100_CFG_MS_TIMEOUT  50 /* Max 255 */
/* Note: Debug flag disabled (commented out) by default */
//#define TMP100_CFG_DEBUG
#endif

#endif /* _TMP100_PLATFORM_CFG_H_ */
