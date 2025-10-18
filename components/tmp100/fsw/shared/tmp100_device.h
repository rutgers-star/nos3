/*******************************************************************************
** File: tmp100_device.h
**
** Purpose:
**   This is the header file for the TMP100 device.
**
*******************************************************************************/
#ifndef _TMP100_DEVICE_H_
#define _TMP100_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "hwlib.h"

#ifndef TMP100_CFG
#include "tmp100_platform_cfg.h"
#endif

/*
** TMP100 Register Addresses (per TI datasheet)
*/
#define TMP100_REG_TEMPERATURE  0x00
#define TMP100_REG_CONFIG       0x01
#define TMP100_REG_TLOW         0x02
#define TMP100_REG_THIGH        0x03

/*
** TMP100 device housekeeping telemetry definition
*/
typedef struct
{
    uint16_t TemperatureRaw;    /* Raw 16-bit temperature register (12-bit data in upper bits) */
    uint16_t ConfigRegister;    /* Configuration register value */
    uint16_t TlowRegister;      /* TLOW register value */
    uint16_t ThighRegister;     /* THIGH register value */

} __attribute__((packed)) TMP100_Device_HK_tlm_t;
#define TMP100_DEVICE_HK_LNGTH sizeof(TMP100_Device_HK_tlm_t)

/*
** TMP100 device data telemetry definition
*/
typedef struct
{
    uint16_t TemperatureRaw;    /* Raw 16-bit temperature register */
    int16_t  TemperatureCounts; /* Signed 12-bit temperature counts */
    float    TemperatureCelsius;/* Temperature in degrees Celsius */

} __attribute__((packed)) TMP100_Device_Data_tlm_t;
#define TMP100_DEVICE_DATA_LNGTH sizeof(TMP100_Device_Data_tlm_t)

/*
** Prototypes
*/
int32_t TMP100_ReadTemperature(i2c_bus_info_t *device, TMP100_Device_HK_tlm_t *data);
int32_t TMP100_ReadAllRegisters(i2c_bus_info_t *device, TMP100_Device_HK_tlm_t *data);
int32_t TMP100_WriteConfig(i2c_bus_info_t *device, uint8_t config_value);
int32_t TMP100_RequestData(i2c_bus_info_t *device, TMP100_Device_Data_tlm_t *data);

/*
** Temperature conversion helper
** Converts TMP100 raw register value to degrees Celsius
*/
static inline float TMP100_RawToCelsius(uint16_t raw_temp)
{
    /* TMP100 uses 12-bit two's complement in upper 12 bits of 16-bit register */
    int16_t temp_counts = (int16_t)(raw_temp) >> 4;  /* Extract 12-bit value */
    return (float)temp_counts * 0.0625f;              /* Convert to Celsius (0.0625°C per LSB) */
}

#endif /* _TMP100_DEVICE_H_ */
