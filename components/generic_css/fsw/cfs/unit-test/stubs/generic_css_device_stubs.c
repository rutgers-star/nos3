#include "utgenstub.h"
#include "generic_css_device.h"

/*
 * ----------------------------------------------------
 * Generated stub function for GENERIC_CSS_RequestData()
 * ----------------------------------------------------
 */
int32_t GENERIC_CSS_RequestData(i2c_bus_info_t *device, GENERIC_CSS_Device_Data_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(GENERIC_CSS_RequestData, int32_t);

    UT_GenStub_AddParam(GENERIC_CSS_RequestData, i2c_bus_info_t *, device);
    UT_GenStub_AddParam(GENERIC_CSS_RequestData, GENERIC_CSS_Device_Data_tlm_t *, data);

    UT_GenStub_Execute(GENERIC_CSS_RequestData, Basic, NULL);

    return UT_GenStub_GetReturnValue(GENERIC_CSS_RequestData, int32_t);
}
