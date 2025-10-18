#ifndef _GENERIC_mag_CHECKOUT_DEVICE_CFG_H_
#define _GENERIC_mag_CHECKOUT_DEVICE_CFG_H_

/*
** GENERIC_mag Checkout Configuration
*/
#define GENERIC_mag_CFG
/* Notes:
**   NOS3 spi requires matching handle and bus number
*/
#define GENERIC_MAG_CFG_STRING        "spi_2"
#define GENERIC_MAG_CFG_HANDLE        2
#define GENERIC_MAG_CFG_DELAY         32 /* SPI transaction delay, in microseconds */
#define GENERIC_MAG_CFG_BAUD          1000000
#define GENERIC_MAG_CFG_SPI_MODE      1
#define GENERIC_MAG_CFG_BITS_PER_WORD 8
#define GENERIC_MAG_CFG_BUS           0
#define GENERIC_MAG_CFG_CS            2
/* Note: Debug flag disabled (commented out) by default */
#define GENERIC_MAG_CFG_DEBUG
#endif /* _GENERIC_mag_CHECKOUT_DEVICE_CFG_H_ */
