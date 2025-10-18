/*******************************************************************************
** File: generic_torquer_device_stubs.c
**
** Purpose:
**   This is the header file for the GENERIC_TORQUER device.
**
*******************************************************************************/

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in generic_torquer_device header
 */

/* Use properly scoped include paths that will work with the build system */
#include "generic_torquer_device.h"
#include "libtrq.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for GENERIC_TORQUER_Config()
 * ----------------------------------------------------
 */
int32_t GENERIC_TORQUER_Config(GENERIC_TORQUER_Device_tlm_t *trqHk, trq_info_t *trqDevice, uint8_t percent, uint8_t dir)
{
    UT_GenStub_SetupReturnBuffer(GENERIC_TORQUER_Config, int32_t);

    UT_GenStub_AddParam(GENERIC_TORQUER_Config, GENERIC_TORQUER_Device_tlm_t *, trqHk);
    UT_GenStub_AddParam(GENERIC_TORQUER_Config, trq_info_t *, trqDevice);
    UT_GenStub_AddParam(GENERIC_TORQUER_Config, uint8_t, percent);
    UT_GenStub_AddParam(GENERIC_TORQUER_Config, uint8_t, dir);

    UT_GenStub_Execute(GENERIC_TORQUER_Config, Basic, NULL);

    return UT_GenStub_GetReturnValue(GENERIC_TORQUER_Config, int32_t);
}
