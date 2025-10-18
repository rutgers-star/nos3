#include "utgenstub.h"
#include "sample_device.h"

int32_t SAMPLE_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length)
{
    UT_GenStub_SetupReturnBuffer(SAMPLE_ReadData, int32_t);

    UT_GenStub_AddParam(SAMPLE_ReadData, uart_info_t *, device);
    UT_GenStub_AddParam(SAMPLE_ReadData, uint8_t *, read_data);
    UT_GenStub_AddParam(SAMPLE_ReadData, uint8_t, data_length);

    UT_GenStub_Execute(SAMPLE_ReadData, Basic, NULL);

    return UT_GenStub_GetReturnValue(SAMPLE_ReadData, int32_t);
}

int32_t SAMPLE_CommandDevice(uart_info_t *device, uint8_t cmd, uint32_t payload)
{
    UT_GenStub_SetupReturnBuffer(SAMPLE_CommandDevice, int32_t);

    UT_GenStub_AddParam(SAMPLE_CommandDevice, uart_info_t *, device);
    UT_GenStub_AddParam(SAMPLE_CommandDevice, uint8_t, cmd);
    UT_GenStub_AddParam(SAMPLE_CommandDevice, uint32_t, payload);

    UT_GenStub_Execute(SAMPLE_CommandDevice, Basic, NULL);

    return UT_GenStub_GetReturnValue(SAMPLE_CommandDevice, int32_t);
}

int32_t SAMPLE_RequestHK(uart_info_t *device, SAMPLE_Device_HK_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(SAMPLE_RequestHK, int32_t);

    UT_GenStub_AddParam(SAMPLE_RequestHK, uart_info_t *, device);
    UT_GenStub_AddParam(SAMPLE_RequestHK, SAMPLE_Device_HK_tlm_t *, data);

    UT_GenStub_Execute(SAMPLE_RequestHK, Basic, NULL);

    return UT_GenStub_GetReturnValue(SAMPLE_RequestHK, int32_t);
}

int32_t SAMPLE_RequestData(uart_info_t *device, SAMPLE_Device_Data_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(SAMPLE_RequestData, int32_t);

    UT_GenStub_AddParam(SAMPLE_RequestData, uart_info_t *, device);
    UT_GenStub_AddParam(SAMPLE_RequestData, SAMPLE_Device_Data_tlm_t *, data);

    UT_GenStub_Execute(SAMPLE_RequestData, Basic, NULL);

    return UT_GenStub_GetReturnValue(SAMPLE_RequestData, int32_t);
}
