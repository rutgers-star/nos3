/************************************************************************
** File:
**    raspberry_pi_events.h
**
** Purpose:
**  Define RASPBERRY_PI application event IDs
**
*************************************************************************/

#ifndef _RASPBERRY_PI_EVENTS_H_
#define _RASPBERRY_PI_EVENTS_H_

/* Standard app event IDs */
#define RASPBERRY_PI_RESERVED_EID        0
#define RASPBERRY_PI_STARTUP_INF_EID     1
#define RASPBERRY_PI_LEN_ERR_EID         2
#define RASPBERRY_PI_PIPE_ERR_EID        3
#define RASPBERRY_PI_SUB_CMD_ERR_EID     4
#define RASPBERRY_PI_SUB_REQ_HK_ERR_EID  5
#define RASPBERRY_PI_PROCESS_CMD_ERR_EID 6

/* Standard command event IDs */
#define RASPBERRY_PI_CMD_ERR_EID         10
#define RASPBERRY_PI_CMD_NOOP_INF_EID    11
#define RASPBERRY_PI_CMD_RESET_INF_EID   12
#define RASPBERRY_PI_CMD_ENABLE_INF_EID  13
#define RASPBERRY_PI_ENABLE_INF_EID      14
#define RASPBERRY_PI_ENABLE_ERR_EID      15
#define RASPBERRY_PI_CMD_DISABLE_INF_EID 16
#define RASPBERRY_PI_DISABLE_INF_EID     17
#define RASPBERRY_PI_DISABLE_ERR_EID     18

/* Device specific command event IDs */
#define RASPBERRY_PI_CMD_CONFIG_EN_ERR_EID  20
#define RASPBERRY_PI_CMD_CONFIG_VAL_ERR_EID 21
#define RASPBERRY_PI_CMD_CONFIG_INF_EID     22
#define RASPBERRY_PI_CMD_CONFIG_DEV_ERR_EID 23

/* Standard telemetry event IDs */
#define RASPBERRY_PI_DEVICE_TLM_ERR_EID 30
#define RASPBERRY_PI_REQ_HK_ERR_EID     31

/* Device specific telemetry event IDs */
#define RASPBERRY_PI_REQ_DATA_ERR_EID        32
#define RASPBERRY_PI_REQ_DATA_STATUS_ERR_EID 33

/* Hardware protocol event IDs */
#define RASPBERRY_PI_UART_INIT_ERR_EID  40
#define RASPBERRY_PI_UART_CLOSE_ERR_EID 41

#endif /* _RASPBERRY_PI_EVENTS_H_ */
