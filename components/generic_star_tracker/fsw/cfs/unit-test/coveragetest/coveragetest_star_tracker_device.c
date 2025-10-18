#include "generic_star_tracker_app_coveragetest_common.h"

void Test_GENERIC_STAR_TRACKER_ReadData(void)
{
    uart_info_t device;
    uint8_t     read_data[GENERIC_STAR_TRACKER_DEVICE_DATA_SIZE];
    uint8_t     data_length = GENERIC_STAR_TRACKER_DEVICE_DATA_SIZE;

    /* Test timeout case */
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), GENERIC_STAR_TRACKER_CFG_MS_TIMEOUT + 1, 0);
    GENERIC_STAR_TRACKER_ReadData(&device, read_data, data_length);

    /* Test normal case with exact bytes */
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, data_length);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, data_length);
    GENERIC_STAR_TRACKER_ReadData(&device, read_data, data_length);

    /* Test case with more bytes available than requested */
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, data_length + 1);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, data_length);
    GENERIC_STAR_TRACKER_ReadData(&device, read_data, data_length);

    /* Test read error case */
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, data_length);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, data_length - 1);
    GENERIC_STAR_TRACKER_ReadData(&device, read_data, data_length);
}

void Test_GENERIC_STAR_TRACKER_CommandDevice(void)
{
    uart_info_t device;
    uint8_t     cmd_code = GENERIC_STAR_TRACKER_DEVICE_REQ_HK_CMD;
    uint32_t    payload  = 0;
    uint8_t     echo_data[GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE];
    uint8_t     write_data[GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE];

    /* Prepare test data */
    write_data[0] = GENERIC_STAR_TRACKER_DEVICE_HDR_0;
    write_data[1] = GENERIC_STAR_TRACKER_DEVICE_HDR_1;
    write_data[2] = cmd_code;
    write_data[3] = payload >> 24;
    write_data[4] = payload >> 16;
    write_data[5] = payload >> 8;
    write_data[6] = payload;
    write_data[7] = GENERIC_STAR_TRACKER_DEVICE_TRAILER_0;
    write_data[8] = GENERIC_STAR_TRACKER_DEVICE_TRAILER_1;

    /* Test flush error */
    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_ERROR);
    GENERIC_STAR_TRACKER_CommandDevice(&device, cmd_code, payload);

    /* Test write error */
    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE - 1);
    GENERIC_STAR_TRACKER_CommandDevice(&device, cmd_code, payload);

    /* Test normal case with echo match */
    memcpy(echo_data, write_data, sizeof(write_data));
    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE);
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE);
    UT_SetDataBuffer(UT_KEY(uart_read_port), echo_data, sizeof(echo_data), false);
    GENERIC_STAR_TRACKER_CommandDevice(&device, cmd_code, payload);

    /* Test echo mismatch */
    echo_data[0] = 0xFF; /* Mismatch first byte */
    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE);
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE);
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE);
    UT_SetDataBuffer(UT_KEY(uart_read_port), echo_data, sizeof(echo_data), false);
    GENERIC_STAR_TRACKER_CommandDevice(&device, cmd_code, payload);

    /* Test read error */
    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, GENERIC_STAR_TRACKER_DEVICE_CMD_SIZE);
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_ReadData), 1, OS_ERROR);
    GENERIC_STAR_TRACKER_CommandDevice(&device, cmd_code, payload);
}

void Test_GENERIC_STAR_TRACKER_RequestHK(void)
{
    uart_info_t                          device;
    GENERIC_STAR_TRACKER_Device_HK_tlm_t data;

    static uint8_t read_data[] = {0xDE, 0xAD, 0x0, 0x0, 0x0, 0x1, 0xBE, 0xEF};
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, sizeof(read_data));
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, sizeof(read_data));
    UT_SetDataBuffer(UT_KEY(uart_read_port), &read_data, sizeof(read_data), false);
    GENERIC_STAR_TRACKER_RequestHK(&device, &data);
}

void Test_GENERIC_STAR_TRACKER_RequestData(void)
{
    uart_info_t                            device;
    GENERIC_STAR_TRACKER_Device_Data_tlm_t data;

    /* Test command error */
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_CommandDevice), 1, OS_ERROR);
    GENERIC_STAR_TRACKER_RequestData(&device, &data);

    /* Test read error */
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_CommandDevice), 1, OS_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_ReadData), 1, OS_ERROR);
    GENERIC_STAR_TRACKER_RequestData(&device, &data);

    /* Test normal case with valid data */
    uint8_t read_data[GENERIC_STAR_TRACKER_DEVICE_DATA_SIZE];

    /* Set header bytes */
    read_data[0] = GENERIC_STAR_TRACKER_DEVICE_HDR_0; /* 0xDE */
    read_data[1] = GENERIC_STAR_TRACKER_DEVICE_HDR_1; /* 0xAD */

    /* Set quaternion values */
    read_data[2] = 0x7F;
    read_data[3] = 0xFF; /* Q0: 32767 (max positive) */
    read_data[4] = 0x7F;
    read_data[5] = 0xFF; /* Q1: 32767 (max positive) */
    read_data[6] = 0x7F;
    read_data[7] = 0xFF; /* Q2: 32767 (max positive) */
    read_data[8] = 0x7F;
    read_data[9] = 0xFF; /* Q3: 32767 (max positive) */

    /* Set IsValid flag */
    read_data[10] = 0x01;

    /* Set trailer bytes */
    read_data[11] = GENERIC_STAR_TRACKER_DEVICE_TRAILER_0; /* 0xBE */
    read_data[12] = GENERIC_STAR_TRACKER_DEVICE_TRAILER_1; /* 0xEF */

    /* Set up the test data for both CommandDevice and ReadData */
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_CommandDevice), 1, OS_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, sizeof(read_data));
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, sizeof(read_data));
    UT_SetDataBuffer(UT_KEY(uart_read_port), read_data, sizeof(read_data), false);
    UT_SetDataBuffer(UT_KEY(GENERIC_STAR_TRACKER_ReadData), read_data, sizeof(read_data), false);
    GENERIC_STAR_TRACKER_RequestData(&device, &data);

    /* Test case with minimum quaternion values */
    read_data[2] = 0x80;
    read_data[3] = 0x00; /* Q0: -32768 (min negative) */
    read_data[4] = 0x80;
    read_data[5] = 0x00; /* Q1: -32768 (min negative) */
    read_data[6] = 0x80;
    read_data[7] = 0x00; /* Q2: -32768 (min negative) */
    read_data[8] = 0x80;
    read_data[9] = 0x00; /* Q3: -32768 (min negative) */
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_CommandDevice), 1, OS_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, sizeof(read_data));
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, sizeof(read_data));
    UT_SetDataBuffer(UT_KEY(uart_read_port), read_data, sizeof(read_data), false);
    UT_SetDataBuffer(UT_KEY(GENERIC_STAR_TRACKER_ReadData), read_data, sizeof(read_data), false);
    GENERIC_STAR_TRACKER_RequestData(&device, &data);

    /* Test case with zero quaternion values */
    read_data[2] = 0x00;
    read_data[3] = 0x00; /* Q0: 0 */
    read_data[4] = 0x00;
    read_data[5] = 0x00; /* Q1: 0 */
    read_data[6] = 0x00;
    read_data[7] = 0x00; /* Q2: 0 */
    read_data[8] = 0x00;
    read_data[9] = 0x00; /* Q3: 0 */
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_CommandDevice), 1, OS_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_bytes_available), 1, sizeof(read_data));
    UT_SetDeferredRetcode(UT_KEY(uart_read_port), 1, sizeof(read_data));
    UT_SetDataBuffer(UT_KEY(uart_read_port), read_data, sizeof(read_data), false);
    UT_SetDataBuffer(UT_KEY(GENERIC_STAR_TRACKER_ReadData), read_data, sizeof(read_data), false);
    GENERIC_STAR_TRACKER_RequestData(&device, &data);

    /* Test invalid header/trailer */
    read_data[0] = 0xFF; /* Invalid header */
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_CommandDevice), 1, OS_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(GENERIC_STAR_TRACKER_ReadData), 1, OS_SUCCESS);
    UT_SetDataBuffer(UT_KEY(uart_read_port), read_data, sizeof(read_data), false);
    UT_SetDataBuffer(UT_KEY(GENERIC_STAR_TRACKER_ReadData), read_data, sizeof(read_data), false);
    GENERIC_STAR_TRACKER_RequestData(&device, &data);
}
void Test_GENERIC_STAR_TRACKER_RequestData_Hook(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context,
                                                va_list va)
{
}

void Test_GENERIC_STAR_TRACKER_RequestHK_Hook(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context,
                                              va_list va)
{
}

/*
 * Setup function prior to every test
 */
void Generic_star_tracker_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Generic_star_tracker_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UT_SetVaHandlerFunction(UT_KEY(Test_GENERIC_STAR_TRACKER_RequestData), Test_GENERIC_STAR_TRACKER_RequestData_Hook,
                            NULL);
    UT_SetVaHandlerFunction(UT_KEY(Test_GENERIC_STAR_TRACKER_RequestHK), Test_GENERIC_STAR_TRACKER_RequestHK_Hook,
                            NULL);
    ADD_TEST(GENERIC_STAR_TRACKER_ReadData);
    ADD_TEST(GENERIC_STAR_TRACKER_CommandDevice);
    ADD_TEST(GENERIC_STAR_TRACKER_RequestHK);
    ADD_TEST(GENERIC_STAR_TRACKER_RequestData);
}