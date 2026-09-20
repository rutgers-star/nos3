/*******************************************************************************
** File: thermal_control_events.h
**
** Purpose:
**  Define Thermal Control application event IDs
**
*******************************************************************************/

#ifndef _THERMAL_CONTROL_EVENTS_H_
#define _THERMAL_CONTROL_EVENTS_H_

/*
** Event IDs
*/
#define THERMAL_RESERVED_EID          0
#define THERMAL_STARTUP_INF_EID       1
#define THERMAL_COMMAND_ERR_EID       2
#define THERMAL_COMMANDNOP_INF_EID    3
#define THERMAL_COMMANDRST_INF_EID    4
#define THERMAL_INVALID_MSGID_ERR_EID 5
#define THERMAL_LEN_ERR_EID           6
#define THERMAL_PIPE_ERR_EID          7

/* Thermal control specific events */
#define THERMAL_ENABLED_INF_EID       10
#define THERMAL_DISABLED_INF_EID      11
#define THERMAL_HEATER_ON_INF_EID     12
#define THERMAL_HEATER_OFF_INF_EID    13
#define THERMAL_THRESHOLD_SET_INF_EID 14
#define THERMAL_THRESHOLD_ERR_EID     15
#define THERMAL_TEMP_UPDATE_INF_EID   16
#define THERMAL_EPS_CMD_ERR_EID       17

#define THERMAL_EVENT_COUNTS 18

#endif /* _THERMAL_CONTROL_EVENTS_H_ */
