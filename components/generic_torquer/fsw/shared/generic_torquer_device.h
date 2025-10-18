/*******************************************************************************
** File: generic_torquer_device.h
**
** Purpose:
**   This is the header file for the GENERIC_TORQUER device.
**
*******************************************************************************/
#ifndef _GENERIC_TORQUER_DEVICE_H_
#define _GENERIC_TORQUER_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "hwlib.h"
#include "generic_torquer_platform_cfg.h"

/*
** Type definitions
*/
#define GENERIC_TORQUER_DEVICE_DISABLED 0
#define GENERIC_TORQUER_DEVICE_ENABLED  1

/*
** GENERIC_TORQUER device telemetry definition
*/
typedef struct
{
    uint8_t Direction;
    uint8_t PercentOn;

} __attribute__((packed)) GENERIC_TORQUER_Device_tlm_t;
#define GENERIC_TORQUER_DEVICE_TLM_LNGTH sizeof(GENERIC_TORQUER_Device_tlm_t)

/*
** Prototypes
*/
int32_t GENERIC_TORQUER_Config(GENERIC_TORQUER_Device_tlm_t *trqHk, trq_info_t *trqDevice, uint8_t percent,
                               uint8_t dir);

#endif /* _GENERIC_TORQUER_DEVICE_H_ */
