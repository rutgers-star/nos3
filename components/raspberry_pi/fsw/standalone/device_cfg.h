#ifndef _RASPBERRY_PI_CHECKOUT_DEVICE_CFG_H_
#define _RASPBERRY_PI_CHECKOUT_DEVICE_CFG_H_

/*
** RASPBERRY_PI Checkout Configuration
*/
#define RASPBERRY_PI_CFG
/* Note: NOS3 uart requires matching handle and bus number */
#define RASPBERRY_PI_CFG_STRING      "/dev/usart_16"
#define RASPBERRY_PI_CFG_HANDLE      16
#define RASPBERRY_PI_CFG_BAUDRATE_HZ 115200
#define RASPBERRY_PI_CFG_MS_TIMEOUT  250
#define RASPBERRY_PI_CFG_DEBUG

#endif /* _RASPBERRY_PI_CHECKOUT_DEVICE_CFG_H_ */
