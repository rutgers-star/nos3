#ifndef _GENERIC_imu_CHECKOUT_DEVICE_CFG_H_
#define _GENERIC_imu_CHECKOUT_DEVICE_CFG_H_

/*
** GENERIC_imu Checkout Configuration
*/
#define GENERIC_imu_CFG
/* Note: NOS3 uart requires matching handle and bus number */
#define GENERIC_IMU_CFG_STRING      "/dev/usart_16"
#define GENERIC_IMU_CFG_HANDLE      0
#define GENERIC_IMU_CFG_CAN_ID      15
#define GENERIC_IMU_CFG_BAUDRATE_HZ 115200
#define GENERIC_IMU_CFG_MS_TIMEOUT  50 /* Max 255 */

#define GENERIC_IMU_CFG_CAN_BITRATE    1000000
#define GENERIC_IMU_CFG_CAN_TIMEOUT    1
#define GENERIC_IMU_CFG_CAN_MS_TIMEOUT 100
#define GENERIC_IMU_CFG_CAN_XFER_US    5000
#define GENERIC_IMU_CFG_RETRY_ATTEMPTS 3
#define GENERIC_imu_CFG_DEBUG

#endif /* _GENERIC_imu_CHECKOUT_DEVICE_CFG_H_ */
