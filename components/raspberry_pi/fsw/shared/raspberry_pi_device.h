/*******************************************************************************
** File: raspberry_pi_device.h
**
** Purpose:
**   This is the header file for the RASPBERRY_PI device.
**
*******************************************************************************/
#ifndef _RASPBERRY_PI_DEVICE_H_
#define _RASPBERRY_PI_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "hwlib.h"

#ifndef RASPBERRY_PI_CFG
#include "raspberry_pi_platform_cfg.h"
#endif

/*
** Type definitions
** TODO: Make specific to your application
*/
#define RASPBERRY_PI_DEVICE_HDR   0xDEAD
#define RASPBERRY_PI_DEVICE_HDR_0 0xDE
#define RASPBERRY_PI_DEVICE_HDR_1 0xAD

#define RASPBERRY_PI_DEVICE_NOOP_CMD     0x00
#define RASPBERRY_PI_DEVICE_REQ_HK_CMD   0x01
#define RASPBERRY_PI_DEVICE_REQ_DATA_CMD 0x02
#define RASPBERRY_PI_DEVICE_CFG_CMD      0x03

#define RASPBERRY_PI_DEVICE_TRAILER   0xBEEF
#define RASPBERRY_PI_DEVICE_TRAILER_0 0xBE
#define RASPBERRY_PI_DEVICE_TRAILER_1 0xEF

#define RASPBERRY_PI_DEVICE_HDR_TRL_LEN 4
#define RASPBERRY_PI_DEVICE_CMD_SIZE    9

/*
** RASPBERRY_PI device housekeeping telemetry definition
*/
typedef struct
{
    uint32_t DeviceCounter;
    uint32_t DeviceConfig;
    uint32_t DeviceStatus;

} __attribute__((packed)) RASPBERRY_PI_Device_HK_tlm_t;
#define RASPBERRY_PI_DEVICE_HK_LNGTH sizeof(RASPBERRY_PI_Device_HK_tlm_t)
#define RASPBERRY_PI_DEVICE_HK_SIZE  RASPBERRY_PI_DEVICE_HK_LNGTH + RASPBERRY_PI_DEVICE_HDR_TRL_LEN

/*
** RASPBERRY_PI device data telemetry definition
*/
typedef struct
{
    uint32_t DeviceCounter;
    uint16_t DeviceDataX;
    uint16_t DeviceDataY;
    uint16_t DeviceDataZ;

} __attribute__((packed)) RASPBERRY_PI_Device_Data_tlm_t;
#define RASPBERRY_PI_DEVICE_DATA_LNGTH sizeof(RASPBERRY_PI_Device_Data_tlm_t)
#define RASPBERRY_PI_DEVICE_DATA_SIZE  RASPBERRY_PI_DEVICE_DATA_LNGTH + RASPBERRY_PI_DEVICE_HDR_TRL_LEN

/*
** Prototypes
*/
int32_t RASPBERRY_PI_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length);
int32_t RASPBERRY_PI_CommandDevice(uart_info_t *device, uint8_t cmd, uint32_t payload);
int32_t RASPBERRY_PI_RequestHK(uart_info_t *device, RASPBERRY_PI_Device_HK_tlm_t *data);
int32_t RASPBERRY_PI_RequestData(uart_info_t *device, RASPBERRY_PI_Device_Data_tlm_t *data);

#endif /* _RASPBERRY_PI_DEVICE_H_ */
