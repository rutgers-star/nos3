#ifndef _ARDUCAM_CHECKOUT_DEVICE_CFG_H_
#define _ARDUCAM_CHECKOUT_DEVICE_CFG_H_

/*
** Default CAM Configuration
*/
#define CAM_CFG
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
#define FILE_MODE

#endif /* _ARDUCAM_CHECKOUT_DEVICE_CFG_H_ */