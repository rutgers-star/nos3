/************************************************************************
** File:
**    tmp100_events.h
**
** Purpose:
**  Define TMP100 application event IDs
**
*************************************************************************/

#ifndef _TMP100_EVENTS_H_
#define _TMP100_EVENTS_H_

/* Standard app event IDs */
#define TMP100_RESERVED_EID        0
#define TMP100_STARTUP_INF_EID     1
#define TMP100_LEN_ERR_EID         2
#define TMP100_PIPE_ERR_EID        3
#define TMP100_SUB_CMD_ERR_EID     4
#define TMP100_SUB_REQ_HK_ERR_EID  5
#define TMP100_PROCESS_CMD_ERR_EID 6

/* Standard command event IDs */
#define TMP100_CMD_ERR_EID         10
#define TMP100_CMD_NOOP_INF_EID    11
#define TMP100_CMD_RESET_INF_EID   12
#define TMP100_CMD_ENABLE_INF_EID  13
#define TMP100_ENABLE_INF_EID      14
#define TMP100_ENABLE_ERR_EID      15
#define TMP100_CMD_DISABLE_INF_EID 16
#define TMP100_DISABLE_INF_EID     17
#define TMP100_DISABLE_ERR_EID     18

/* Device specific command event IDs */
#define TMP100_CMD_CONFIG_EN_ERR_EID  20
#define TMP100_CMD_CONFIG_VAL_ERR_EID 21
#define TMP100_CMD_CONFIG_INF_EID     22
#define TMP100_CMD_CONFIG_DEV_ERR_EID 23

/* Standard telemetry event IDs */
#define TMP100_DEVICE_TLM_ERR_EID 30
#define TMP100_REQ_HK_ERR_EID     31

/* Device specific telemetry event IDs */
#define TMP100_REQ_DATA_ERR_EID        32
#define TMP100_REQ_DATA_STATUS_ERR_EID 33

/* Hardware protocol event IDs */
#define TMP100_UART_INIT_ERR_EID  40
#define TMP100_UART_CLOSE_ERR_EID 41

#endif /* _TMP100_EVENTS_H_ */
