/************************************************************************
** File:
**    syn_events.h
**
** Purpose:
**  Define SYN application event IDs
**
*************************************************************************/

#ifndef _SYN_EVENTS_H_
#define _SYN_EVENTS_H_

/* Standard app event IDs */
#define SYN_RESERVED_EID              0
#define SYN_STARTUP_INF_EID           1
#define SYN_LEN_ERR_EID               2
#define SYN_PIPE_ERR_EID              3
#define SYN_SUB_CMD_ERR_EID           4
#define SYN_SUB_REQ_HK_ERR_EID        5
#define SYN_PROCESS_CMD_ERR_EID       6

/* Standard command event IDs */
#define SYN_CMD_ERR_EID               10
#define SYN_CMD_NOOP_INF_EID          11
#define SYN_CMD_RESET_INF_EID         12
//#define SYN_CMD_ENABLE_INF_EID        13
#define SYN_ENABLE_INF_EID            14
#define SYN_ENABLE_ERR_EID            15
//#define SYN_CMD_DISABLE_INF_EID       16
#define SYN_DISABLE_INF_EID           17
#define SYN_DISABLE_ERR_EID           18

/* Device specific command event IDs */
//#define SYN_CMD_CONFIG_INF_EID        20

/* Standard telemetry event IDs */
#define SYN_DEVICE_TLM_ERR_EID        30
#define SYN_REQ_HK_ERR_EID            31

/* Device specific telemetry event IDs */
#define SYN_REQ_DATA_ERR_EID          32

/* Hardware protocol event IDs */
#define SYN_UART_INIT_ERR_EID         40
#define SYN_UART_CLOSE_ERR_EID        41

/* SYN Component Event IDs*/ 
#define SYN_CMD_RESET_DEMO_EID        45 //(Should start at 50?)
#define SYN_CMD_PRIO_EID              46
#define SYN_CMD_GET_PDATA_EID         47
#define SYN_CMD_DISP_PDATA_EID        48
#define SYN_CMD_CONFIG_ALPHA_EID      49
#define SYN_CMD_CONFIG_DL_EID         50
#define SYN_CMD_ADD_DATA_EID          51

#endif /* _SYN_EVENTS_H_ */
