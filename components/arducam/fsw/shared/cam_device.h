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

#ifndef _cam_device_h_
#define _cam_device_h_

#include "device_cfg.h"
#include "hwlib.h"
#include "cam_platform_cfg.h"
#include "cam_registers.h"

/************************************************************************
** Debug Definitions
*************************************************************************/
#ifdef FILE_MODE
#define FILE_OUTPUT
#endif
//#define STF1_DEBUG

/************************************************************************
** Type Definitions
*************************************************************************/

// Hardware Specific Definitions
#ifdef OV2640
#define CAM_ADDR      0x30
#define CHIPID_HIGH   0x0A
#define CHIPID_LOW    0x0B
#define CAM_VID       0x26
#define CAM_PID       0x42
#define MAX_FIFO_SIZE 0x5FFFF // 384KByte
#endif
#ifdef OV5640
#define CAM_ADDR      0x3C
#define CHIPID_HIGH   0x300A
#define CHIPID_LOW    0x300B
#define CAM_VID       0x56
#define CAM_PID       0x40
#define MAX_FIFO_SIZE 0x7FFFFF // 8MByte
#endif
#ifdef OV5642
#define CAM_ADDR      0x3C
#define CHIPID_HIGH   0x300A
#define CHIPID_LOW    0x300B
#define CAM_VID       0x56
#define CAM_PID       0x42
#define MAX_FIFO_SIZE 0x7FFFFF // 8MByte
#endif

#define size_160x120   0
#define size_320x240   1
#define size_800x600   2
#define size_1600x1200 3
#define size_2592x1944 4

#define CAM_RUN         0
#define CAM_PAUSE       1
#define CAM_STOP        2
#define CAM_TIME        3
#define CAM_LOW_VOLTAGE 4

/****************************************************/
/* Sensor related definition 						*/
/****************************************************/
#define BMP  0
#define JPEG 1

#define OV2640_160x120   0 // 160x120
#define OV2640_176x144   1 // 176x144
#define OV2640_320x240   2 // 320x240
#define OV2640_352x288   3 // 352x288
#define OV2640_640x480   4 // 640x480
#define OV2640_800x600   5 // 800x600
#define OV2640_1024x768  6 // 1024x768
#define OV2640_1280x1024 7 // 1280x1024
#define OV2640_1600x1200 8 // 1600x1200

/****************************************************/
/* ArduChip related definition 						*/
/****************************************************/
#define ARDUCHIP_MODE 0x02 // Mode register

/*************************************************************************
** Global Data
*************************************************************************/
extern i2c_bus_info_t CAM_I2C;
extern spi_info_t     CAM_SPI;

/*************************************************************************
** Exported Functions
*************************************************************************/
extern int32_t CAM_init_i2c(void);
extern int32_t CAM_init_spi(void);
extern int32_t CAM_config(void);
extern int32_t CAM_capture_prep(void);
extern int32_t CAM_capture(void);
extern int32_t CAM_read_fifo_length(uint32_t *length);
extern int32_t CAM_read_prep(char *buf, uint16_t *i);
extern int32_t CAM_read(char *buf, uint16_t *i, uint8_t *status);
int            take_picture(uint8_t size);

#endif /* _cam_device_h_ */

/************************/
/*  End of File Comment */
/************************/