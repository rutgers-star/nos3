/*******************************************************************************
** File: arducam_checkout.h
**
** Purpose:
**   This is the header file for the ARDUCAM checkout.
**
*******************************************************************************/
#ifndef _ARDUCAM_CHECKOUT_H_
#define _ARDUCAM_CHECKOUT_H_

/*
** Includes
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#include "hwlib.h"
#include "device_cfg.h"
#include "cam_device.h"
#include "cam_registers.h"

#if TGTNAME == cpu1
#include "nos_link.h"
#endif

/*
** Standard Defines
*/
#define PROMPT               "arducam> "
#define MAX_INPUT_BUF        512
#define MAX_INPUT_TOKENS     64
#define MAX_INPUT_TOKEN_SIZE 50
#define TELEM_BUF_LEN        8

/*
** Command Defines
*/
#define CMD_UNKNOWN -1
#define CMD_HELP    0
#define CMD_EXIT    1
#define CMD_I2C     2
#define CMD_SPI     3
#define CMD_NOOP    4
#define CMD_SMALL   5
#define CMD_MEDIUM  6
#define CMD_LARGE   7

/*
** Prototypes
*/
void print_help(void);
int  get_command(const char *str);
int  process_command(int cc, int num_tokens, char tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE]);
int  main(int argc, char *argv[]);

/*
** Generic Prototypes
*/
int  check_number_arguments(int actual, int expected);
void to_lower(char *str);

#endif /* _ARDUCAM_CHECKOUT_H_ */