/*******************************************************************************
** File: generic_torquer_device.c
**
** Purpose:
**   This file contains the source code for the GENERIC_TORQUER device.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_torquer_device.h"

/*
** Configure torquer using hwlib
*/
int32_t GENERIC_TORQUER_Config(GENERIC_TORQUER_Device_tlm_t *trqHk, trq_info_t *trqDevice, uint8_t percent, uint8_t dir)
{
    int32_t status = OS_SUCCESS;

    status = trq_command(trqDevice, percent, dir);
    if (status == OS_SUCCESS)
    {
        trqHk->PercentOn = percent;
        trqHk->Direction = dir;
    }
    return status;
}
