/* Copyright (C) 2009 - 2020 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S.
Government. */

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in libtrq header
 */

#include "libtrq.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for trq_close()
 * ----------------------------------------------------
 */
void trq_close(trq_info_t *device)
{
    UT_GenStub_AddParam(trq_close, trq_info_t *, device);

    UT_GenStub_Execute(trq_close, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for trq_command()
 * ----------------------------------------------------
 */
int32_t trq_command(trq_info_t *device, uint8_t percent_high, bool pos_dir)
{
    UT_GenStub_SetupReturnBuffer(trq_command, int32_t);

    UT_GenStub_AddParam(trq_command, trq_info_t *, device);
    UT_GenStub_AddParam(trq_command, uint8_t, percent_high);
    UT_GenStub_AddParam(trq_command, bool, pos_dir);

    UT_GenStub_Execute(trq_command, Basic, NULL);

    return UT_GenStub_GetReturnValue(trq_command, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for trq_dummy()
 * ----------------------------------------------------
 */
void trq_dummy(void)
{

    UT_GenStub_Execute(trq_dummy, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for trq_init()
 * ----------------------------------------------------
 */
int32_t trq_init(trq_info_t *device)
{
    UT_GenStub_SetupReturnBuffer(trq_init, int32_t);

    UT_GenStub_AddParam(trq_init, trq_info_t *, device);

    UT_GenStub_Execute(trq_init, Basic, NULL);

    return UT_GenStub_GetReturnValue(trq_init, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for trq_set_direction()
 * ----------------------------------------------------
 */
int32_t trq_set_direction(trq_info_t *device, bool direction)
{
    UT_GenStub_SetupReturnBuffer(trq_set_direction, int32_t);

    UT_GenStub_AddParam(trq_set_direction, trq_info_t *, device);
    UT_GenStub_AddParam(trq_set_direction, bool, direction);

    UT_GenStub_Execute(trq_set_direction, Basic, NULL);

    return UT_GenStub_GetReturnValue(trq_set_direction, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for trq_set_period()
 * ----------------------------------------------------
 */
int32_t trq_set_period(trq_info_t *device)
{
    UT_GenStub_SetupReturnBuffer(trq_set_period, int32_t);

    UT_GenStub_AddParam(trq_set_period, trq_info_t *, device);

    UT_GenStub_Execute(trq_set_period, Basic, NULL);

    return UT_GenStub_GetReturnValue(trq_set_period, int32_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for trq_set_time_high()
 * ----------------------------------------------------
 */
int32_t trq_set_time_high(trq_info_t *device, uint32_t new_time)
{
    UT_GenStub_SetupReturnBuffer(trq_set_time_high, int32_t);

    UT_GenStub_AddParam(trq_set_time_high, trq_info_t *, device);
    UT_GenStub_AddParam(trq_set_time_high, uint32_t, new_time);

    UT_GenStub_Execute(trq_set_time_high, Basic, NULL);

    return UT_GenStub_GetReturnValue(trq_set_time_high, int32_t);
}
