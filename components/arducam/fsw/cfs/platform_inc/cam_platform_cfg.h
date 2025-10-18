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

/************************************************************************
** File:
**   $Id: cam_platform_cfg.h  $
**
** Purpose:
**  Define CAM platform configuation parameters - application definitions
**
** Notes:
**
*************************************************************************/
#ifndef _CAM_PLATFORM_CFG_H_
#define _CAM_PLATFORM_CFG_H_

/*
** Default CAM Configuration
*/
#ifndef CAM_CFG
#define CAM_I2C_BUS               2
#define CAM_SPEED                 1000000
#define CAP_DONE_MASK             0x08
#define CAM_TIMEOUT               100
#define CAM_DATA_SIZE             1010
#define CAM_CHILD_TASK_NAME       "CAM_CHILD_TASK"
#define CAM_CHILD_TASK_STACK_SIZE 2048
#define CAM_CHILD_TASK_PRIORITY   205
#define CAM_MUTEX_NAME            "CAM_MUTEX"
#define CAM_SEM_NAME              "CAM_SEM"
// Select Hardware (only 1)
//#define OV2640
#define OV5640
//#define OV5642
// enable file mode:
#define FILE_MODE
#endif

#endif
