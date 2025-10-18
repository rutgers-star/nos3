/*******************************************************************************
** File: tmp100_device.c
**
** Purpose:
**   This file contains the source code for the TMP100 device.
**
*******************************************************************************/

/*
** Include Files
*/
#include "tmp100_device.h"

/*
** Read TMP100 temperature register via I2C
** Implements TMP100 I2C protocol per TI datasheet
*/
int32_t TMP100_ReadTemperature(i2c_bus_info_t *device, TMP100_Device_HK_tlm_t *data)
{
    int32_t status = OS_SUCCESS;
    uint8_t write_data[1] = {TMP100_REG_TEMPERATURE};  /* Set pointer to temperature register */
    uint8_t read_data[2] = {0};

    /* TMP100 I2C read sequence:
     * 1. Write pointer register (0x00 for temperature)
     * 2. Read 2 bytes (MSB, LSB)
     */
    status = i2c_master_transaction(device, TMP100_CFG_I2C_ADDRESS,
                                    write_data, 1, read_data, 2,
                                    TMP100_CFG_I2C_TIMEOUT);

    if (status == OS_SUCCESS)
    {
        /* Combine MSB and LSB into 16-bit value */
        data->TemperatureRaw = (read_data[0] << 8) | read_data[1];

#ifdef TMP100_CFG_DEBUG
        float temp_c = TMP100_RawToCelsius(data->TemperatureRaw);
        OS_printf("TMP100_ReadTemperature: Raw=0x%04x, Temp=%.2f°C\n",
                  data->TemperatureRaw, temp_c);
#endif
    }
    else
    {
#ifdef TMP100_CFG_DEBUG
        OS_printf("TMP100_ReadTemperature: I2C transaction failed with status %d\n", status);
#endif
    }

    return status;
}

/*
** Read all TMP100 registers via I2C
*/
int32_t TMP100_ReadAllRegisters(i2c_bus_info_t *device, TMP100_Device_HK_tlm_t *data)
{
    int32_t status = OS_SUCCESS;
    uint8_t write_data[1];
    uint8_t read_data[2];

    /* Read Temperature Register */
    write_data[0] = TMP100_REG_TEMPERATURE;
    status = i2c_master_transaction(device, TMP100_CFG_I2C_ADDRESS,
                                    write_data, 1, read_data, 2,
                                    TMP100_CFG_I2C_TIMEOUT);
    if (status == OS_SUCCESS)
    {
        data->TemperatureRaw = (read_data[0] << 8) | read_data[1];
    }

    /* Read Config Register */
    write_data[0] = TMP100_REG_CONFIG;
    status = i2c_master_transaction(device, TMP100_CFG_I2C_ADDRESS,
                                    write_data, 1, read_data, 2,
                                    TMP100_CFG_I2C_TIMEOUT);
    if (status == OS_SUCCESS)
    {
        data->ConfigRegister = (read_data[0] << 8) | read_data[1];
    }

    /* Read TLOW Register */
    write_data[0] = TMP100_REG_TLOW;
    status = i2c_master_transaction(device, TMP100_CFG_I2C_ADDRESS,
                                    write_data, 1, read_data, 2,
                                    TMP100_CFG_I2C_TIMEOUT);
    if (status == OS_SUCCESS)
    {
        data->TlowRegister = (read_data[0] << 8) | read_data[1];
    }

    /* Read THIGH Register */
    write_data[0] = TMP100_REG_THIGH;
    status = i2c_master_transaction(device, TMP100_CFG_I2C_ADDRESS,
                                    write_data, 1, read_data, 2,
                                    TMP100_CFG_I2C_TIMEOUT);
    if (status == OS_SUCCESS)
    {
        data->ThighRegister = (read_data[0] << 8) | read_data[1];
    }

#ifdef TMP100_CFG_DEBUG
    OS_printf("TMP100_ReadAllRegisters: Temp=0x%04x, Config=0x%04x, TLOW=0x%04x, THIGH=0x%04x\n",
              data->TemperatureRaw, data->ConfigRegister,
              data->TlowRegister, data->ThighRegister);
#endif

    return status;
}

/*
** Write TMP100 configuration register via I2C
*/
int32_t TMP100_WriteConfig(i2c_bus_info_t *device, uint8_t config_value)
{
    int32_t status = OS_SUCCESS;
    uint8_t write_data[2];

    /* TMP100 config write: pointer register (0x01) + config byte */
    write_data[0] = TMP100_REG_CONFIG;
    write_data[1] = config_value;

    status = i2c_master_transaction(device, TMP100_CFG_I2C_ADDRESS,
                                    write_data, 2, NULL, 0,
                                    TMP100_CFG_I2C_TIMEOUT);

#ifdef TMP100_CFG_DEBUG
    if (status == OS_SUCCESS)
    {
        OS_printf("TMP100_WriteConfig: Wrote 0x%02x to config register\n", config_value);
    }
    else
    {
        OS_printf("TMP100_WriteConfig: Failed with status %d\n", status);
    }
#endif

    return status;
}

/*
** Request TMP100 data with conversion to engineering units
*/
int32_t TMP100_RequestData(i2c_bus_info_t *device, TMP100_Device_Data_tlm_t *data)
{
    int32_t status = OS_SUCCESS;
    uint8_t write_data[1] = {TMP100_REG_TEMPERATURE};
    uint8_t read_data[2] = {0};

    /* Read temperature register */
    status = i2c_master_transaction(device, TMP100_CFG_I2C_ADDRESS,
                                    write_data, 1, read_data, 2,
                                    TMP100_CFG_I2C_TIMEOUT);

    if (status == OS_SUCCESS)
    {
        /* Store raw value */
        data->TemperatureRaw = (read_data[0] << 8) | read_data[1];

        /* Extract 12-bit signed temperature counts */
        data->TemperatureCounts = (int16_t)(data->TemperatureRaw) >> 4;

        /* Convert to Celsius */
        data->TemperatureCelsius = TMP100_RawToCelsius(data->TemperatureRaw);

#ifdef TMP100_CFG_DEBUG
        OS_printf("TMP100_RequestData: Raw=0x%04x, Counts=%d, Temp=%.2f°C\n",
                  data->TemperatureRaw, data->TemperatureCounts,
                  data->TemperatureCelsius);
#endif
    }

    return status;
}
