#ifndef _PAYLOAD_IF_CHECKOUT_DEVICE_CFG_H_
#define _PAYLOAD_IF_CHECKOUT_DEVICE_CFG_H_

/*
** PAYLOAD_IF Checkout Configuration
*/
#define PAYLOAD_IF_CFG
/* Note: NOS3 uart requires matching handle and bus number */
#define PAYLOAD_IF_CFG_STRING      "/dev/usart_17"
#define PAYLOAD_IF_CFG_HANDLE      17
#define PAYLOAD_IF_CFG_BAUDRATE_HZ 115200
#define PAYLOAD_IF_CFG_MS_TIMEOUT  250
#define PAYLOAD_IF_CFG_DEBUG

#endif /* _PAYLOAD_IF_CHECKOUT_DEVICE_CFG_H_ */
