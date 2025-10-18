#ifndef _GENERIC_FSS_CHECKOUT_DEVICE_CFG_H_
#define _GENERIC_FSS_CHECKOUT_DEVICE_CFG_H_

/*
** GENERIC_FSS Checkout Configuration
*/
#define GENERIC_FSS_CFG
/* Note: NOS3 SPI requires matching handle and bus number */
#define GENERIC_FSS_CFG_STRING        "spi_1"
#define GENERIC_FSS_CFG_HANDLE        1
#define GENERIC_FSS_CFG_DELAY         32 /* SPI transaction delay, in microseconds */
#define GENERIC_FSS_CFG_BAUD          1000000
#define GENERIC_FSS_CFG_SPI_MODE      1
#define GENERIC_FSS_CFG_BITS_PER_WORD 8
#define GENERIC_FSS_CFG_BUS           0
#define GENERIC_FSS_CFG_CS            1

#define GENERIC_FSS_CFG_DEBUG

#endif /* _GENERIC_FSS_CHECKOUT_DEVICE_CFG_H_ */
