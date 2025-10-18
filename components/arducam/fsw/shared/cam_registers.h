/* Copyright (C) 2009 - 2017 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S.
Government.

This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including,
but not limited to, any warranty that the software will conform to, specifications any implied warranties of
merchantability, fitness for a particular purpose, and freedom from infringement, and any warranty that the
documentation will conform to the program, or any warranty that the software will be error free.

In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or
consequential damages, arising out of, resulting from, or in any way connected with the software or its documentation.
Whether or not based upon warranty, contract, tort or otherwise, and whether or not loss was sustained from, or arose
out of the results of, or use of, the software, documentation or services provided hereunder

ITC Team
NASA IV&V
ivv-itc@lists.nasa.gov
*/

#ifndef _cam_registers_h_
#define _cam_registers_h_

#include "device_cfg.h"
#include "hwlib.h"
#include "cam_platform_cfg.h"
#include "cam_device.h"

struct sensor_reg
{
    uint16_t reg;
    uint16_t val;
};

extern int32_t CAM_jpeg_init(void);
extern int32_t CAM_yuv422(void);
extern int32_t CAM_jpeg(void);
extern int32_t CAM_jpeg_320x240(void);
extern int32_t CAM_setup(void);
extern int32_t CAM_setSize(uint8_t size);

#endif /* _cam_registers_h_ */