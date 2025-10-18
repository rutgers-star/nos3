
#include "raspberry_pi_app_coveragetest_common.h"

void Test_RASPBERRY_PI_ReadData(void)
{
    uart_info_t device;
    uint8_t     read_data[8];
    uint8_t     data_length = 8;
    RASPBERRY_PI_ReadData(&device, read_data, data_length);

    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, data_length);
    RASPBERRY_PI_ReadData(&device, read_data, data_length);

    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, data_length + 1);
    RASPBERRY_PI_ReadData(&device, read_data, data_length);
}

void Test_RASPBERRY_PI_CommandDevice(void)
{
    uart_info_t device;
    uint8_t     cmd_code = 0;
    uint32_t    payload  = 0;
    RASPBERRY_PI_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_ERROR);
    RASPBERRY_PI_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, RASPBERRY_PI_DEVICE_CMD_SIZE);
    RASPBERRY_PI_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, RASPBERRY_PI_DEVICE_CMD_SIZE);
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, 9);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, 9);
    UT_SetDefaultReturnValue(UT_KEY(RASPBERRY_PI_ReadData), OS_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(RASPBERRY_PI_ReadData), 1, OS_SUCCESS);
    RASPBERRY_PI_CommandDevice(&device, cmd_code, payload);
}

void Test_RASPBERRY_PI_RequestHK(void)
{
    uart_info_t            device;
    RASPBERRY_PI_Device_HK_tlm_t data;
    RASPBERRY_PI_RequestHK(&device, &data);

    uint8_t read_data[] = {0xDE, 0xAD, 0x00, 0x00, 0x00, 0x07, 0x00, 0x06,
                           0x00, 0x0C, 0x00, 0x12, 0x00, 0x00, 0xBE, 0xEF};
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, 16);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, 16);
    UT_SetDataBuffer(UT_KEY(uart_read_port), &read_data, sizeof(read_data), false);
    RASPBERRY_PI_RequestHK(&device, &data);

    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, OS_ERROR);
    RASPBERRY_PI_RequestHK(&device, &data);
}

void Test_RASPBERRY_PI_RequestData(void)
{
    uart_info_t              device;
    RASPBERRY_PI_Device_Data_tlm_t data;
    RASPBERRY_PI_RequestData(&device, &data);

    uint8_t read_data[] = {0xDE, 0xAD, 0x00, 0x00, 0x00, 0x07, 0x00, 0x06,
                           0x00, 0x0C, 0x00, 0x12, 0x00, 0x00, 0xBE, 0xEF};
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, 16);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, 16);
    UT_SetDataBuffer(UT_KEY(uart_read_port), &read_data, sizeof(read_data), false);
    RASPBERRY_PI_RequestData(&device, &data);

    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, OS_ERROR);
    RASPBERRY_PI_RequestData(&device, &data);
}

void Test_RASPBERRY_PI_RequestData_Hook(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va) {}

/*
 * Setup function prior to every test
 */
void Raspberry_pi_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Raspberry_pi_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UT_SetVaHandlerFunction(UT_KEY(Test_RASPBERRY_PI_RequestData), Test_RASPBERRY_PI_RequestData_Hook, NULL);
    ADD_TEST(RASPBERRY_PI_ReadData);
    ADD_TEST(RASPBERRY_PI_CommandDevice);
    ADD_TEST(RASPBERRY_PI_RequestHK);
    ADD_TEST(RASPBERRY_PI_RequestData);
}