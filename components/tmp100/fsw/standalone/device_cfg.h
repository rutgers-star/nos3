#ifndef _TMP100_CHECKOUT_DEVICE_CFG_H_
#define _TMP100_CHECKOUT_DEVICE_CFG_H_

/*
** TMP100 Checkout Configuration
*/
#define TMP100_CFG
/* Note: NOS3 uart requires matching handle and bus number */
#define TMP100_CFG_STRING      "/dev/usart_16"
#define TMP100_CFG_HANDLE      16
#define TMP100_CFG_BAUDRATE_HZ 115200
#define TMP100_CFG_MS_TIMEOUT  250
#define TMP100_CFG_DEBUG

#endif /* _TMP100_CHECKOUT_DEVICE_CFG_H_ */
