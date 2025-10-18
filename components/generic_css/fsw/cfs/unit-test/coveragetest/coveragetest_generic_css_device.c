#include "generic_css_app_coveragetest_common.h"

void Test_GENERIC_CSS_RequestData(void)
{
    i2c_bus_info_t                device;
    GENERIC_CSS_Device_Data_tlm_t data;

    /* Test basic case with minimal setup */
    GENERIC_CSS_RequestData(&device, &data);

    /* Test successful case */
    uint8_t read_data[] = {
        0x01, 0x02, /* Voltage[0] = 0x0102 */
        0x03, 0x04, /* Voltage[1] = 0x0304 */
        0x05, 0x06, /* Voltage[2] = 0x0506 */
        0x07, 0x08, /* Voltage[3] = 0x0708 */
        0x09, 0x0A, /* Voltage[4] = 0x090A */
        0x0B, 0x0C  /* Voltage[5] = 0x0B0C */
    };

    /* Setup for successful I2C transaction */
    UT_SetDefaultReturnValue(UT_KEY(i2c_master_transaction), OS_SUCCESS);
    UT_SetDataBuffer(UT_KEY(i2c_master_transaction), read_data, sizeof(read_data), false);
    GENERIC_CSS_RequestData(&device, &data);

    /* Test error case */
    UT_SetDefaultReturnValue(UT_KEY(i2c_master_transaction), OS_ERROR);
    GENERIC_CSS_RequestData(&device, &data);
}

void Test_GENERIC_CSS_RequestData_Hook(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context,
                                       va_list va)
{
}

/*
 * Setup function prior to every test
 */
void Generic_css_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Generic_css_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UT_SetVaHandlerFunction(UT_KEY(Test_GENERIC_CSS_RequestData), Test_GENERIC_CSS_RequestData_Hook, NULL);
    ADD_TEST(GENERIC_CSS_RequestData);
}