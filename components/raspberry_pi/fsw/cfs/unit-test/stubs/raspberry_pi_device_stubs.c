#include "utgenstub.h"
#include "raspberry_pi_device.h"

int32_t RASPBERRY_PI_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length)
{
    UT_GenStub_SetupReturnBuffer(RASPBERRY_PI_ReadData, int32_t);

    UT_GenStub_AddParam(RASPBERRY_PI_ReadData, uart_info_t *, device);
    UT_GenStub_AddParam(RASPBERRY_PI_ReadData, uint8_t *, read_data);
    UT_GenStub_AddParam(RASPBERRY_PI_ReadData, uint8_t, data_length);

    UT_GenStub_Execute(RASPBERRY_PI_ReadData, Basic, NULL);

    return UT_GenStub_GetReturnValue(RASPBERRY_PI_ReadData, int32_t);
}

int32_t RASPBERRY_PI_CommandDevice(uart_info_t *device, uint8_t cmd, uint32_t payload)
{
    UT_GenStub_SetupReturnBuffer(RASPBERRY_PI_CommandDevice, int32_t);

    UT_GenStub_AddParam(RASPBERRY_PI_CommandDevice, uart_info_t *, device);
    UT_GenStub_AddParam(RASPBERRY_PI_CommandDevice, uint8_t, cmd);
    UT_GenStub_AddParam(RASPBERRY_PI_CommandDevice, uint32_t, payload);

    UT_GenStub_Execute(RASPBERRY_PI_CommandDevice, Basic, NULL);

    return UT_GenStub_GetReturnValue(RASPBERRY_PI_CommandDevice, int32_t);
}

int32_t RASPBERRY_PI_RequestHK(uart_info_t *device, RASPBERRY_PI_Device_HK_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(RASPBERRY_PI_RequestHK, int32_t);

    UT_GenStub_AddParam(RASPBERRY_PI_RequestHK, uart_info_t *, device);
    UT_GenStub_AddParam(RASPBERRY_PI_RequestHK, RASPBERRY_PI_Device_HK_tlm_t *, data);

    UT_GenStub_Execute(RASPBERRY_PI_RequestHK, Basic, NULL);

    return UT_GenStub_GetReturnValue(RASPBERRY_PI_RequestHK, int32_t);
}

int32_t RASPBERRY_PI_RequestData(uart_info_t *device, RASPBERRY_PI_Device_Data_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(RASPBERRY_PI_RequestData, int32_t);

    UT_GenStub_AddParam(RASPBERRY_PI_RequestData, uart_info_t *, device);
    UT_GenStub_AddParam(RASPBERRY_PI_RequestData, RASPBERRY_PI_Device_Data_tlm_t *, data);

    UT_GenStub_Execute(RASPBERRY_PI_RequestData, Basic, NULL);

    return UT_GenStub_GetReturnValue(RASPBERRY_PI_RequestData, int32_t);
}
