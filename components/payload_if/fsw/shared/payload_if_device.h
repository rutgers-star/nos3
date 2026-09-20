/*******************************************************************************
** File: payload_if_device.h
**
** Purpose:
**   This is the header file for the PAYLOAD_IF device.
**
*******************************************************************************/
#ifndef _PAYLOAD_IF_DEVICE_H_
#define _PAYLOAD_IF_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "hwlib.h"

#ifndef PAYLOAD_IF_CFG
#include "payload_if_platform_cfg.h"
#endif

/*
** Type definitions
** TODO: Make specific to your application
*/
#define PAYLOAD_IF_DEVICE_HDR   0xDEAD
#define PAYLOAD_IF_DEVICE_HDR_0 0xDE
#define PAYLOAD_IF_DEVICE_HDR_1 0xAD

#define PAYLOAD_IF_DEVICE_NOOP_CMD     0x00
#define PAYLOAD_IF_DEVICE_REQ_HK_CMD   0x01
#define PAYLOAD_IF_DEVICE_REQ_DATA_CMD 0x02
#define PAYLOAD_IF_DEVICE_CFG_CMD      0x03

#define PAYLOAD_IF_DEVICE_TRAILER   0xBEEF
#define PAYLOAD_IF_DEVICE_TRAILER_0 0xBE
#define PAYLOAD_IF_DEVICE_TRAILER_1 0xEF

#define PAYLOAD_IF_DEVICE_HDR_TRL_LEN 4
#define PAYLOAD_IF_DEVICE_CMD_SIZE    9

/*
** PAYLOAD_IF device housekeeping telemetry definition
*/
typedef struct
{
    uint32_t DeviceCounter;
    uint32_t DeviceConfig;
    uint32_t DeviceStatus;

} __attribute__((packed)) PAYLOAD_IF_Device_HK_tlm_t;
#define PAYLOAD_IF_DEVICE_HK_LNGTH sizeof(PAYLOAD_IF_Device_HK_tlm_t)
#define PAYLOAD_IF_DEVICE_HK_SIZE  PAYLOAD_IF_DEVICE_HK_LNGTH + PAYLOAD_IF_DEVICE_HDR_TRL_LEN

/*
** PAYLOAD_IF device data telemetry definition
*/
typedef struct
{
    uint32_t DeviceCounter;
    uint16_t DeviceDataX;
    uint16_t DeviceDataY;
    uint16_t DeviceDataZ;

} __attribute__((packed)) PAYLOAD_IF_Device_Data_tlm_t;
#define PAYLOAD_IF_DEVICE_DATA_LNGTH sizeof(PAYLOAD_IF_Device_Data_tlm_t)
#define PAYLOAD_IF_DEVICE_DATA_SIZE  PAYLOAD_IF_DEVICE_DATA_LNGTH + PAYLOAD_IF_DEVICE_HDR_TRL_LEN

/*
** Prototypes
*/
int32_t PAYLOAD_IF_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length);
int32_t PAYLOAD_IF_CommandDevice(uart_info_t *device, uint8_t cmd, uint32_t payload);
int32_t PAYLOAD_IF_RequestHK(uart_info_t *device, PAYLOAD_IF_Device_HK_tlm_t *data);
int32_t PAYLOAD_IF_RequestData(uart_info_t *device, PAYLOAD_IF_Device_Data_tlm_t *data);

#endif /* _PAYLOAD_IF_DEVICE_H_ */
