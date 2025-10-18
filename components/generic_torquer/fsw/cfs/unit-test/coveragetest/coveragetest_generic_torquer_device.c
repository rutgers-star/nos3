#include "generic_torquer_app_coveragetest_common.h"

void Test_GENERIC_TORQUER_Config(void)
{
    GENERIC_TORQUER_Device_tlm_t trqHk;
    trq_info_t                   trqDevice;
    int32_t                      status;
    uint8_t                      percent = 50;
    uint8_t                      dir     = 0;

    /* Initialize the telemetry structure */
    memset(&trqHk, 0, sizeof(trqHk));

    /* Test success case */
    UT_SetDeferredRetcode(UT_KEY(trq_command), 1, OS_SUCCESS);
    status = GENERIC_TORQUER_Config(&trqHk, &trqDevice, percent, dir);
    UtAssert_True(status == OS_SUCCESS, "GENERIC_TORQUER_Config returned OS_SUCCESS");
    UtAssert_True(trqHk.PercentOn == percent, "trqHk.PercentOn == %d", percent);
    UtAssert_True(trqHk.Direction == dir, "trqHk.Direction == %d", dir);

    /* Test error case */
    UT_SetDeferredRetcode(UT_KEY(trq_command), 1, OS_ERROR);
    status = GENERIC_TORQUER_Config(&trqHk, &trqDevice, percent, dir);
    UtAssert_True(status == OS_ERROR, "GENERIC_TORQUER_Config returned OS_ERROR");
    UtAssert_True(trqHk.PercentOn == percent, "trqHk.PercentOn unchanged after error");
    UtAssert_True(trqHk.Direction == dir, "trqHk.Direction unchanged after error");

    /* Test different values */
    percent = 75;
    dir     = 1;
    UT_SetDeferredRetcode(UT_KEY(trq_command), 1, OS_SUCCESS);
    status = GENERIC_TORQUER_Config(&trqHk, &trqDevice, percent, dir);
    UtAssert_True(status == OS_SUCCESS, "GENERIC_TORQUER_Config returned OS_SUCCESS");
    UtAssert_True(trqHk.PercentOn == percent, "trqHk.PercentOn == %d", percent);
    UtAssert_True(trqHk.Direction == dir, "trqHk.Direction == %d", dir);

    /* Test boundary values */
    percent = 0;
    dir     = 0;
    UT_SetDeferredRetcode(UT_KEY(trq_command), 1, OS_SUCCESS);
    status = GENERIC_TORQUER_Config(&trqHk, &trqDevice, percent, dir);
    UtAssert_True(status == OS_SUCCESS, "GENERIC_TORQUER_Config returned OS_SUCCESS");
    UtAssert_True(trqHk.PercentOn == percent, "trqHk.PercentOn == %d", percent);
    UtAssert_True(trqHk.Direction == dir, "trqHk.Direction == %d", dir);

    percent = 100;
    dir     = 1;
    UT_SetDeferredRetcode(UT_KEY(trq_command), 1, OS_SUCCESS);
    status = GENERIC_TORQUER_Config(&trqHk, &trqDevice, percent, dir);
    UtAssert_True(status == OS_SUCCESS, "GENERIC_TORQUER_Config returned OS_SUCCESS");
    UtAssert_True(trqHk.PercentOn == percent, "trqHk.PercentOn == %d", percent);
    UtAssert_True(trqHk.Direction == dir, "trqHk.Direction == %d", dir);
}

void Test_GENERIC_TORQUER_Config_Hook(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
}

/*
 * Setup function prior to every test
 */
void Generic_torquer_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Generic_torquer_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UT_SetVaHandlerFunction(UT_KEY(Test_GENERIC_TORQUER_Config), Test_GENERIC_TORQUER_Config_Hook, NULL);
    ADD_TEST(GENERIC_TORQUER_Config);
}