#include "libi2c.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for i2c_master_close()
 * ----------------------------------------------------
 */
int32_t i2c_master_close(i2c_bus_info_t *device)
{
    UT_GenStub_SetupReturnBuffer(i2c_master_close, int32_t);

    UT_GenStub_AddParam(i2c_master_close, i2c_bus_info_t *, device);

    UT_GenStub_Execute(i2c_master_close, Basic, NULL);

    return UT_GenStub_GetReturnValue(i2c_master_close, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for i2c_master_init()
 * ----------------------------------------------------
 */
int32_t i2c_master_init(i2c_bus_info_t *device)
{
    UT_GenStub_SetupReturnBuffer(i2c_master_init, int32_t);

    UT_GenStub_AddParam(i2c_master_init, i2c_bus_info_t *, device);

    UT_GenStub_Execute(i2c_master_init, Basic, NULL);

    return UT_GenStub_GetReturnValue(i2c_master_init, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for i2c_master_transaction()
 * ----------------------------------------------------
 */
int32_t i2c_master_transaction(i2c_bus_info_t *device, uint8_t addr, void *txbuf, uint8_t txlen, void *rxbuf,
                               uint8_t rxlen, uint16_t timeout)
{
    UT_GenStub_SetupReturnBuffer(i2c_master_transaction, int32_t);

    UT_GenStub_AddParam(i2c_master_transaction, i2c_bus_info_t *, device);
    UT_GenStub_AddParam(i2c_master_transaction, uint8_t, addr);
    UT_GenStub_AddParam(i2c_master_transaction, void *, txbuf);
    UT_GenStub_AddParam(i2c_master_transaction, uint8_t, txlen);
    UT_GenStub_AddParam(i2c_master_transaction, void *, rxbuf);
    UT_GenStub_AddParam(i2c_master_transaction, uint8_t, rxlen);
    UT_GenStub_AddParam(i2c_master_transaction, uint16_t, timeout);

    UT_GenStub_Execute(i2c_master_transaction, Basic, NULL);

    return UT_GenStub_GetReturnValue(i2c_master_transaction, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for i2c_multiple_transaction()
 * ----------------------------------------------------
 */
int32_t i2c_multiple_transaction(i2c_bus_info_t *device, uint8_t addr, struct i2c_rdwr_ioctl_data *rdwr_data,
                                 uint16_t timeout)
{
    UT_GenStub_SetupReturnBuffer(i2c_multiple_transaction, int32_t);

    UT_GenStub_AddParam(i2c_multiple_transaction, i2c_bus_info_t *, device);
    UT_GenStub_AddParam(i2c_multiple_transaction, uint8_t, addr);
    UT_GenStub_AddParam(i2c_multiple_transaction, struct i2c_rdwr_ioctl_data *, rdwr_data);
    UT_GenStub_AddParam(i2c_multiple_transaction, uint16_t, timeout);

    UT_GenStub_Execute(i2c_multiple_transaction, Basic, NULL);

    return UT_GenStub_GetReturnValue(i2c_multiple_transaction, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for i2c_read_transaction()
 * ----------------------------------------------------
 */
int32_t i2c_read_transaction(i2c_bus_info_t *device, uint8_t addr, void *rxbuf, uint8_t rxlen, uint8_t timeout)
{
    UT_GenStub_SetupReturnBuffer(i2c_read_transaction, int32_t);

    UT_GenStub_AddParam(i2c_read_transaction, i2c_bus_info_t *, device);
    UT_GenStub_AddParam(i2c_read_transaction, uint8_t, addr);
    UT_GenStub_AddParam(i2c_read_transaction, void *, rxbuf);
    UT_GenStub_AddParam(i2c_read_transaction, uint8_t, rxlen);
    UT_GenStub_AddParam(i2c_read_transaction, uint8_t, timeout);

    UT_GenStub_Execute(i2c_read_transaction, Basic, NULL);

    return UT_GenStub_GetReturnValue(i2c_read_transaction, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for i2c_write_transaction()
 * ----------------------------------------------------
 */
int32_t i2c_write_transaction(i2c_bus_info_t *device, uint8_t addr, void *txbuf, uint8_t txlen, uint8_t timeout)
{
    UT_GenStub_SetupReturnBuffer(i2c_write_transaction, int32_t);

    UT_GenStub_AddParam(i2c_write_transaction, i2c_bus_info_t *, device);
    UT_GenStub_AddParam(i2c_write_transaction, uint8_t, addr);
    UT_GenStub_AddParam(i2c_write_transaction, void *, txbuf);
    UT_GenStub_AddParam(i2c_write_transaction, uint8_t, txlen);
    UT_GenStub_AddParam(i2c_write_transaction, uint8_t, timeout);

    UT_GenStub_Execute(i2c_write_transaction, Basic, NULL);

    return UT_GenStub_GetReturnValue(i2c_write_transaction, int32_t);
}
