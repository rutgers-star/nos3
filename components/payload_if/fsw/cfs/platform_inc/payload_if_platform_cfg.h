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
#define PAYLOAD_IF_CFG_STRING      "usart_17"
#define PAYLOAD_IF_CFG_HANDLE      17
#define PAYLOAD_IF_CFG_BAUDRATE_HZ 115200
#define PAYLOAD_IF_CFG_MS_TIMEOUT  50 /* Max 255 */

/*
** Asynchronous UART receive task
*/
#define PAYLOAD_IF_RX_TASK_NAME          "PAYLOAD_IF_RX"
#define PAYLOAD_IF_RX_TASK_STACK_SIZE    2048
#define PAYLOAD_IF_RX_TASK_PRIORITY      80
#define PAYLOAD_IF_RX_TASK_MS_DELAY      10
#define PAYLOAD_IF_RX_EXIT_SEM_NAME      "PAYLOAD_IF_RXSEM"
#define PAYLOAD_IF_RX_TASK_STOP_MS       1000 /* Disable waits this long for the RX task to stop */

/*
** Periodic housekeeping, driven by a once-per-second tick from SCH and
** changeable with PAYLOAD_IF_SET_HK_PERIOD_CC (0 disables it)
*/
#define PAYLOAD_IF_HK_PERIOD_SEC_DEFAULT 5
#define PAYLOAD_IF_HK_PERIOD_SEC_MAX     3600
/* Note: Debug flag disabled (commented out) by default */
//#define PAYLOAD_IF_CFG_DEBUG
#endif

#endif /* _PAYLOAD_IF_PLATFORM_CFG_H_ */
