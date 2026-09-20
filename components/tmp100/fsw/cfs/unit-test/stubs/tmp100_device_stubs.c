#include "utgenstub.h"
#include "tmp100_device.h"

int32_t TMP100_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length)
{
    UT_GenStub_SetupReturnBuffer(TMP100_ReadData, int32_t);

    UT_GenStub_AddParam(TMP100_ReadData, uart_info_t *, device);
    UT_GenStub_AddParam(TMP100_ReadData, uint8_t *, read_data);
    UT_GenStub_AddParam(TMP100_ReadData, uint8_t, data_length);

    UT_GenStub_Execute(TMP100_ReadData, Basic, NULL);

    return UT_GenStub_GetReturnValue(TMP100_ReadData, int32_t);
}

int32_t TMP100_CommandDevice(uart_info_t *device, uint8_t cmd, uint32_t payload)
{
    UT_GenStub_SetupReturnBuffer(TMP100_CommandDevice, int32_t);

    UT_GenStub_AddParam(TMP100_CommandDevice, uart_info_t *, device);
    UT_GenStub_AddParam(TMP100_CommandDevice, uint8_t, cmd);
    UT_GenStub_AddParam(TMP100_CommandDevice, uint32_t, payload);

    UT_GenStub_Execute(TMP100_CommandDevice, Basic, NULL);

    return UT_GenStub_GetReturnValue(TMP100_CommandDevice, int32_t);
}

int32_t TMP100_RequestHK(uart_info_t *device, TMP100_Device_HK_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(TMP100_RequestHK, int32_t);

    UT_GenStub_AddParam(TMP100_RequestHK, uart_info_t *, device);
    UT_GenStub_AddParam(TMP100_RequestHK, TMP100_Device_HK_tlm_t *, data);

    UT_GenStub_Execute(TMP100_RequestHK, Basic, NULL);

    return UT_GenStub_GetReturnValue(TMP100_RequestHK, int32_t);
}

int32_t TMP100_RequestData(uart_info_t *device, TMP100_Device_Data_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(TMP100_RequestData, int32_t);

    UT_GenStub_AddParam(TMP100_RequestData, uart_info_t *, device);
    UT_GenStub_AddParam(TMP100_RequestData, TMP100_Device_Data_tlm_t *, data);

    UT_GenStub_Execute(TMP100_RequestData, Basic, NULL);

    return UT_GenStub_GetReturnValue(TMP100_RequestData, int32_t);
}
