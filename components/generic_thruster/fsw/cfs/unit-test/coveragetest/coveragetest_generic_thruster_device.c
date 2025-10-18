#include "generic_thruster_app_coveragetest_common.h"

void Test_GENERIC_THRUSTER_SetPercentage(void)
{
    uart_info_t device;
    int32_t     status;
    uint8_t     thruster_number = 1;
    uint8_t     percentage      = 50;
    uint8_t     data_length     = GENERIC_THRUSTER_DEVICE_CMD_SIZE;

    /* Initialize the device structure */
    memset(&device, 0, sizeof(device));

    /* Test success case */
    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, GENERIC_THRUSTER_DEVICE_CMD_SIZE);
    status = GENERIC_THRUSTER_SetPercentage(&device, thruster_number, percentage, data_length);
    UtAssert_True(status == OS_SUCCESS, "GENERIC_THRUSTER_SetPercentage returned OS_SUCCESS");

    /* Test success case */
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, GENERIC_THRUSTER_DEVICE_CMD_SIZE);
    status = GENERIC_THRUSTER_SetPercentage(&device, thruster_number, percentage, data_length);
    UtAssert_True(status == OS_SUCCESS, "GENERIC_THRUSTER_SetPercentage returned OS_SUCCESS");

    /* Test uart_flush failure case */
    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_ERROR);
    status = GENERIC_THRUSTER_SetPercentage(&device, thruster_number, percentage, data_length);
    UtAssert_True(status == UART_ERROR, "GENERIC_THRUSTER_SetPercentage returned UART_ERROR");

    /* Test uart_write_port failure case */
    UT_SetDeferredRetcode(UT_KEY(uart_flush), 1, UART_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, 0);
    status = GENERIC_THRUSTER_SetPercentage(&device, thruster_number, percentage, data_length);
    UtAssert_True(status == OS_ERROR, "GENERIC_THRUSTER_SetPercentage returned OS_ERROR");
}

void Test_GENERIC_THRUSTER_SetPercentage_Hook(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context,
                                              va_list va)
{
}

/*
 * Setup function prior to every test
 */
void Generic_thruster_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Generic_thruster_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UT_SetVaHandlerFunction(UT_KEY(Test_GENERIC_THRUSTER_SetPercentage), Test_GENERIC_THRUSTER_SetPercentage_Hook,
                            NULL);
    ADD_TEST(GENERIC_THRUSTER_SetPercentage);
}