/*******************************************************************************
** File: generic_thruster_device.c
**
** Purpose:
**   This file contains the source code for the GENERIC_THRUSTER device.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_thruster_device.h"

int32_t GENERIC_THRUSTER_SetPercentage(uart_info_t *device, uint8_t thruster_number, uint8_t percentage,
                                       uint8_t data_length)
{
    int32_t  status;
    uint32_t response;
    uint8_t  request[6];

    request[0] = GENERIC_THRUSTER_DEVICE_HDR_0;
    request[1] = GENERIC_THRUSTER_DEVICE_HDR_1;
    request[2] = thruster_number;
    request[3] = percentage;
    request[4] = GENERIC_THRUSTER_DEVICE_TRAILER_0;
    request[5] = GENERIC_THRUSTER_DEVICE_TRAILER_1;
    /* Flush any prior data */
    status = uart_flush(device);
    if (status == UART_SUCCESS)
    {
        /* Write data */
        response = uart_write_port(device, request, GENERIC_THRUSTER_DEVICE_CMD_SIZE);
#ifdef GENERIC_THRUSTER_CFG_DEBUG
        OS_printf("  GENERIC_THRUSTER_SetPercentage[%d] = ", status);
        for (uint32_t i = 0; i < GENERIC_THRUSTER_DEVICE_CMD_SIZE; i++)
        {
            OS_printf("%02x", request[i]);
        }
        OS_printf("\n");
#endif

        if (response != GENERIC_THRUSTER_DEVICE_CMD_SIZE)
        {
            status = OS_ERROR;
        }
        else
        {
            status = OS_SUCCESS;
        }
    } /* uart_flush*/

    return status;
}