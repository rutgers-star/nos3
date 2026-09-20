#include "utgenstub.h"
#include "payload_if_device.h"

int32_t PAYLOAD_IF_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length)
{
    UT_GenStub_SetupReturnBuffer(PAYLOAD_IF_ReadData, int32_t);

    UT_GenStub_AddParam(PAYLOAD_IF_ReadData, uart_info_t *, device);
    UT_GenStub_AddParam(PAYLOAD_IF_ReadData, uint8_t *, read_data);
    UT_GenStub_AddParam(PAYLOAD_IF_ReadData, uint8_t, data_length);

    UT_GenStub_Execute(PAYLOAD_IF_ReadData, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYLOAD_IF_ReadData, int32_t);
}

int32_t PAYLOAD_IF_CommandDevice(uart_info_t *device, uint8_t cmd, uint32_t payload)
{
    UT_GenStub_SetupReturnBuffer(PAYLOAD_IF_CommandDevice, int32_t);

    UT_GenStub_AddParam(PAYLOAD_IF_CommandDevice, uart_info_t *, device);
    UT_GenStub_AddParam(PAYLOAD_IF_CommandDevice, uint8_t, cmd);
    UT_GenStub_AddParam(PAYLOAD_IF_CommandDevice, uint32_t, payload);

    UT_GenStub_Execute(PAYLOAD_IF_CommandDevice, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYLOAD_IF_CommandDevice, int32_t);
}

int32_t PAYLOAD_IF_RequestHK(uart_info_t *device, PAYLOAD_IF_Device_HK_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(PAYLOAD_IF_RequestHK, int32_t);

    UT_GenStub_AddParam(PAYLOAD_IF_RequestHK, uart_info_t *, device);
    UT_GenStub_AddParam(PAYLOAD_IF_RequestHK, PAYLOAD_IF_Device_HK_tlm_t *, data);

    UT_GenStub_Execute(PAYLOAD_IF_RequestHK, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYLOAD_IF_RequestHK, int32_t);
}

int32_t PAYLOAD_IF_RequestData(uart_info_t *device, PAYLOAD_IF_Device_Data_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(PAYLOAD_IF_RequestData, int32_t);

    UT_GenStub_AddParam(PAYLOAD_IF_RequestData, uart_info_t *, device);
    UT_GenStub_AddParam(PAYLOAD_IF_RequestData, PAYLOAD_IF_Device_Data_tlm_t *, data);

    UT_GenStub_Execute(PAYLOAD_IF_RequestData, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYLOAD_IF_RequestData, int32_t);
}
