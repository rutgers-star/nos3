
#include "sample_app_coveragetest_common.h"

void Test_SAMPLE_ReadData(void)
{
    uart_info_t device;
    uint8_t     read_data[8];
    uint8_t     data_length = 8;
    SAMPLE_ReadData(&device, read_data, data_length);

    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, data_length);
    SAMPLE_ReadData(&device, read_data, data_length);

    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, data_length + 1);
    SAMPLE_ReadData(&device, read_data, data_length);
}

void Test_SAMPLE_CommandDevice(void)
{
    uart_info_t device;
    uint8_t     cmd_code = 0;
    uint32_t    payload  = 0;
    SAMPLE_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_ERROR);
    SAMPLE_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, SAMPLE_DEVICE_CMD_SIZE);
    SAMPLE_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, SAMPLE_DEVICE_CMD_SIZE);
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, 9);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, 9);
    UT_SetDefaultReturnValue(UT_KEY(SAMPLE_ReadData), OS_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(SAMPLE_ReadData), 1, OS_SUCCESS);
    SAMPLE_CommandDevice(&device, cmd_code, payload);
}

void Test_SAMPLE_RequestHK(void)
{
    uart_info_t            device;
    SAMPLE_Device_HK_tlm_t data;
    SAMPLE_RequestHK(&device, &data);

    uint8_t read_data[] = {0xDE, 0xAD, 0x00, 0x00, 0x00, 0x07, 0x00, 0x06,
                           0x00, 0x0C, 0x00, 0x12, 0x00, 0x00, 0xBE, 0xEF};
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, 16);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, 16);
    UT_SetDataBuffer(UT_KEY(uart_read_port), &read_data, sizeof(read_data), false);
    SAMPLE_RequestHK(&device, &data);

    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, OS_ERROR);
    SAMPLE_RequestHK(&device, &data);
}

void Test_SAMPLE_RequestData(void)
{
    uart_info_t              device;
    SAMPLE_Device_Data_tlm_t data;
    SAMPLE_RequestData(&device, &data);

    uint8_t read_data[] = {0xDE, 0xAD, 0x00, 0x00, 0x00, 0x07, 0x00, 0x06,
                           0x00, 0x0C, 0x00, 0x12, 0x00, 0x00, 0xBE, 0xEF};
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, 16);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, 16);
    UT_SetDataBuffer(UT_KEY(uart_read_port), &read_data, sizeof(read_data), false);
    SAMPLE_RequestData(&device, &data);

    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, OS_ERROR);
    SAMPLE_RequestData(&device, &data);
}

void Test_SAMPLE_RequestData_Hook(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va) {}

/*
 * Setup function prior to every test
 */
void Sample_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Sample_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UT_SetVaHandlerFunction(UT_KEY(Test_SAMPLE_RequestData), Test_SAMPLE_RequestData_Hook, NULL);
    ADD_TEST(SAMPLE_ReadData);
    ADD_TEST(SAMPLE_CommandDevice);
    ADD_TEST(SAMPLE_RequestHK);
    ADD_TEST(SAMPLE_RequestData);
}