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
#define TMP100_CFG
/* I2C Configuration for TMP100 Temperature Sensor */
#define TMP100_CFG_I2C_HANDLE  3
#define TMP100_CFG_I2C_SPEED   400 /* kHz - Fast mode */
#define TMP100_CFG_I2C_ADDRESS 0x48 /* 7-bit address: ADD0=0, ADD1=0 */
#define TMP100_CFG_I2C_TIMEOUT 10 /* milliseconds */

/* UART/Generic interface configuration (for template compatibility) */
#define TMP100_CFG_STRING        "/dev/usart_29"
#define TMP100_CFG_HANDLE        29
#define TMP100_CFG_BAUDRATE_HZ   115200
#define TMP100_CFG_MS_TIMEOUT    250

/* Note: Debug flag disabled (commented out) by default */
//#define TMP100_CFG_DEBUG
#endif

#endif /* _TMP100_PLATFORM_CFG_H_ */
