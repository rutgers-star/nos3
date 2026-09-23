/************************************************************************
** File:
**    payload_if_events.h
**
** Purpose:
**  Define PAYLOAD_IF application event IDs
**
*************************************************************************/

#ifndef _PAYLOAD_IF_EVENTS_H_
#define _PAYLOAD_IF_EVENTS_H_

/* Standard app event IDs */
#define PAYLOAD_IF_RESERVED_EID        0
#define PAYLOAD_IF_STARTUP_INF_EID     1
#define PAYLOAD_IF_LEN_ERR_EID         2
#define PAYLOAD_IF_PIPE_ERR_EID        3
#define PAYLOAD_IF_SUB_CMD_ERR_EID     4
#define PAYLOAD_IF_SUB_REQ_HK_ERR_EID  5
#define PAYLOAD_IF_PROCESS_CMD_ERR_EID 6

/* Standard command event IDs */
#define PAYLOAD_IF_CMD_ERR_EID         10
#define PAYLOAD_IF_CMD_NOOP_INF_EID    11
#define PAYLOAD_IF_CMD_RESET_INF_EID   12
#define PAYLOAD_IF_CMD_ENABLE_INF_EID  13
#define PAYLOAD_IF_ENABLE_INF_EID      14
#define PAYLOAD_IF_ENABLE_ERR_EID      15
#define PAYLOAD_IF_CMD_DISABLE_INF_EID 16
#define PAYLOAD_IF_DISABLE_INF_EID     17
#define PAYLOAD_IF_DISABLE_ERR_EID     18

/* Device specific command event IDs */
#define PAYLOAD_IF_CMD_CONFIG_EN_ERR_EID  20
#define PAYLOAD_IF_CMD_CONFIG_VAL_ERR_EID 21
#define PAYLOAD_IF_CMD_CONFIG_INF_EID     22
#define PAYLOAD_IF_CMD_CONFIG_DEV_ERR_EID 23

/* Standard telemetry event IDs */
#define PAYLOAD_IF_DEVICE_TLM_ERR_EID 30
#define PAYLOAD_IF_REQ_HK_ERR_EID     31

/* Device specific telemetry event IDs */
#define PAYLOAD_IF_REQ_DATA_ERR_EID        32
#define PAYLOAD_IF_REQ_DATA_STATUS_ERR_EID 33

/* Hardware protocol event IDs */
#define PAYLOAD_IF_UART_INIT_ERR_EID  40
#define PAYLOAD_IF_UART_CLOSE_ERR_EID 41

#endif /* _PAYLOAD_IF_EVENTS_H_ */
