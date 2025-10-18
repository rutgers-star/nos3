/*******************************************************************************
** File: generic_thruster_device_stubs.c
**
** Purpose:
**   This file contains stub functions for the GENERIC_THRUSTER device interface.
**
*******************************************************************************/

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in generic_thruster_device header
 */

#include "generic_thruster_device.h"
#include "libuart.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for GENERIC_THRUSTER_SetPercentage()
 * ----------------------------------------------------
 */
int32_t GENERIC_THRUSTER_SetPercentage(uart_info_t *device, uint8_t thruster_number, uint8_t percentage,
                                       uint8_t data_length)
{
    UT_GenStub_SetupReturnBuffer(GENERIC_THRUSTER_SetPercentage, int32_t);

    UT_GenStub_AddParam(GENERIC_THRUSTER_SetPercentage, uart_info_t *, device);
    UT_GenStub_AddParam(GENERIC_THRUSTER_SetPercentage, uint8_t, thruster_number);
    UT_GenStub_AddParam(GENERIC_THRUSTER_SetPercentage, uint8_t, percentage);
    UT_GenStub_AddParam(GENERIC_THRUSTER_SetPercentage, uint8_t, data_length);

    UT_GenStub_Execute(GENERIC_THRUSTER_SetPercentage, Basic, NULL);

    return UT_GenStub_GetReturnValue(GENERIC_THRUSTER_SetPercentage, int32_t);
}
