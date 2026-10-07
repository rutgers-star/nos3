#include <math.h>
#include <string.h>

#include "utassert.h"
#include "uttest.h"
#include "utstubs.h"

#include "generic_eps_msg.h"
#include "thermal_control_app.h"
#include "tmp100_msg.h"

static void ThermalControl_UT_Setup(void)
{
    UT_ResetState(0);
    memset(&THERMAL_AppData, 0, sizeof(THERMAL_AppData));
    THERMAL_AppData.ControlEnabled    = true;
    THERMAL_AppData.State             = THERMAL_STATE_IDLE;
    THERMAL_AppData.TempLowThreshold  = 20.0;
    THERMAL_AppData.TempHighThreshold = 25.0;
    THERMAL_AppData.HeaterEpsSwitch   = THERMAL_HEATER_EPS_SWITCH;
}

static void ThermalControl_UT_TearDown(void)
{
}

static void Test_THERMAL_Tmp100Conversion(void)
{
    UtAssert_True(fabs(THERMAL_Tmp100RawToCelsius(0x1900) - 25.0) < 0.0001,
                  "0x1900 converts to 25 C");
    UtAssert_True(fabs(THERMAL_Tmp100RawToCelsius(0xF600) + 10.0) < 0.0001,
                  "0xF600 converts to -10 C");
    UtAssert_True(fabs(THERMAL_Tmp100RawToCelsius(0xFFC0) + 0.25) < 0.0001,
                  "0xFFC0 converts to -0.25 C");
}

static void Test_THERMAL_Hysteresis(void)
{
    THERMAL_UpdateControlLoop(19.0);
    UtAssert_True(THERMAL_AppData.HeaterState, "cold sample turns heater on");
    UtAssert_INT32_EQ(THERMAL_AppData.State, THERMAL_STATE_HEATING);
    UtAssert_INT32_EQ(THERMAL_AppData.HeaterOnCount, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);

    THERMAL_UpdateControlLoop(22.0);
    UtAssert_True(THERMAL_AppData.HeaterState, "heater remains on inside hysteresis band");
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);

    THERMAL_UpdateControlLoop(26.0);
    UtAssert_True(!THERMAL_AppData.HeaterState, "hot sample turns heater off");
    UtAssert_INT32_EQ(THERMAL_AppData.State, THERMAL_STATE_COOLING);
    UtAssert_INT32_EQ(THERMAL_AppData.HeaterOffCount, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 2);
}

static void Test_THERMAL_ClosedLoopTelemetry(void)
{
    TMP100_Hk_tlm_t sample;

    memset(&sample, 0, sizeof(sample));
    sample.DeviceHK.TemperatureRaw = 0x0F00; /* 15 C */
    THERMAL_ProcessTelemetry((CFE_SB_Buffer_t *)&sample);

    UtAssert_True(fabs(THERMAL_AppData.CurrentTemperature - 15.0) < 0.0001,
                  "TMP100 telemetry is converted before control");
    UtAssert_True(THERMAL_AppData.HeaterState, "cold telemetry commands EPS heater output on");
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);

    sample.DeviceHK.TemperatureRaw = 0x1E00; /* 30 C */
    THERMAL_ProcessTelemetry((CFE_SB_Buffer_t *)&sample);

    UtAssert_True(fabs(THERMAL_AppData.CurrentTemperature - 30.0) < 0.0001,
                  "second TMP100 sample is converted");
    UtAssert_True(!THERMAL_AppData.HeaterState, "hot telemetry commands EPS heater output off");
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 2);
}

static void Test_THERMAL_DisableIsFailSafe(void)
{
    THERMAL_AppData.HeaterState = true;
    THERMAL_AppData.State       = THERMAL_STATE_HEATING;

    THERMAL_DisableControl();

    UtAssert_True(!THERMAL_AppData.ControlEnabled, "control is disabled");
    UtAssert_True(!THERMAL_AppData.HeaterState, "disable de-energizes heater");
    UtAssert_INT32_EQ(THERMAL_AppData.State, THERMAL_STATE_DISABLED);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
}

void UtTest_Setup(void)
{
    UtTest_Add(Test_THERMAL_Tmp100Conversion, ThermalControl_UT_Setup,
               ThermalControl_UT_TearDown, "THERMAL_Tmp100Conversion");
    UtTest_Add(Test_THERMAL_Hysteresis, ThermalControl_UT_Setup,
               ThermalControl_UT_TearDown, "THERMAL_Hysteresis");
    UtTest_Add(Test_THERMAL_ClosedLoopTelemetry, ThermalControl_UT_Setup,
               ThermalControl_UT_TearDown, "THERMAL_ClosedLoopTelemetry");
    UtTest_Add(Test_THERMAL_DisableIsFailSafe, ThermalControl_UT_Setup,
               ThermalControl_UT_TearDown, "THERMAL_DisableIsFailSafe");
}
