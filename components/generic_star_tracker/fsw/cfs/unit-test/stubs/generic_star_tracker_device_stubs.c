/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in generic_star_tracker_device header
 */

#include "generic_star_tracker_device.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for GENERIC_STAR_TRACKER_CommandDevice()
 * ----------------------------------------------------
 */
int32_t GENERIC_STAR_TRACKER_CommandDevice(uart_info_t *device, uint8_t cmd, uint32_t payload)
{
    UT_GenStub_SetupReturnBuffer(GENERIC_STAR_TRACKER_CommandDevice, int32_t);

    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_CommandDevice, uart_info_t *, device);
    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_CommandDevice, uint8_t, cmd);
    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_CommandDevice, uint32_t, payload);

    UT_GenStub_Execute(GENERIC_STAR_TRACKER_CommandDevice, Basic, NULL);

    return UT_GenStub_GetReturnValue(GENERIC_STAR_TRACKER_CommandDevice, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for GENERIC_STAR_TRACKER_ReadData()
 * ----------------------------------------------------
 */
int32_t GENERIC_STAR_TRACKER_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length)
{
    UT_GenStub_SetupReturnBuffer(GENERIC_STAR_TRACKER_ReadData, int32_t);

    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_ReadData, uart_info_t *, device);
    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_ReadData, uint8_t *, read_data);
    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_ReadData, uint8_t, data_length);

    UT_GenStub_Execute(GENERIC_STAR_TRACKER_ReadData, Basic, NULL);

    return UT_GenStub_GetReturnValue(GENERIC_STAR_TRACKER_ReadData, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for GENERIC_STAR_TRACKER_RequestData()
 * ----------------------------------------------------
 */
int32_t GENERIC_STAR_TRACKER_RequestData(uart_info_t *device, GENERIC_STAR_TRACKER_Device_Data_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(GENERIC_STAR_TRACKER_RequestData, int32_t);

    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_RequestData, uart_info_t *, device);
    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_RequestData, GENERIC_STAR_TRACKER_Device_Data_tlm_t *, data);

    UT_GenStub_Execute(GENERIC_STAR_TRACKER_RequestData, Basic, NULL);

    return UT_GenStub_GetReturnValue(GENERIC_STAR_TRACKER_RequestData, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for GENERIC_STAR_TRACKER_RequestHK()
 * ----------------------------------------------------
 */
int32_t GENERIC_STAR_TRACKER_RequestHK(uart_info_t *device, GENERIC_STAR_TRACKER_Device_HK_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(GENERIC_STAR_TRACKER_RequestHK, int32_t);

    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_RequestHK, uart_info_t *, device);
    UT_GenStub_AddParam(GENERIC_STAR_TRACKER_RequestHK, GENERIC_STAR_TRACKER_Device_HK_tlm_t *, data);

    UT_GenStub_Execute(GENERIC_STAR_TRACKER_RequestHK, Basic, NULL);

    return UT_GenStub_GetReturnValue(GENERIC_STAR_TRACKER_RequestHK, int32_t);
}
