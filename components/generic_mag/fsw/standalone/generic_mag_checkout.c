/*******************************************************************************
** File: generic_mag_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_mag_checkout.h"

/*
** Global Variables
*/
spi_info_t                    Generic_magSpi;
GENERIC_MAG_Device_Data_tlm_t Generic_magData;

/*
** Component Functions
*/
void print_help(void)
{
    printf(PROMPT "command [args]\n"
                  "---------------------------------------------------------------------\n"
                  "help                               - Display help                    \n"
                  "exit                               - Exit app                        \n"
                  "generic_mag                        - Request generic_mag data        \n"
                  "  m                                - ^                               \n"
                  "\n");
}

int get_command(const char *str)
{
    int  status = CMD_UNKNOWN;
    char lcmd[MAX_INPUT_TOKEN_SIZE];
    strncpy(lcmd, str, MAX_INPUT_TOKEN_SIZE);

    /* Convert command to lower case */
    to_lower(lcmd);

    if (strcmp(lcmd, "help") == 0)
    {
        status = CMD_HELP;
    }
    else if (strcmp(lcmd, "exit") == 0)
    {
        status = CMD_EXIT;
    }
    if (strcmp(lcmd, "generic_mag") == 0)
    {
        status = CMD_GENERIC_mag;
    }
    else if (strcmp(lcmd, "m") == 0)
    {
        status = CMD_GENERIC_mag;
    }
    return status;
}

int process_command(int cc, int num_tokens, char tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE])
{
    int32_t status      = OS_SUCCESS;
    int32_t exit_status = OS_SUCCESS;

    /* Process command */
    switch (cc)
    {
        case CMD_HELP:
            print_help();
            break;

        case CMD_EXIT:
            exit_status = OS_ERROR;
            break;

        case CMD_GENERIC_mag:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = GENERIC_MAG_RequestData(&Generic_magSpi, &Generic_magData);
                if (status == OS_SUCCESS)
                {
                    OS_printf("Magnetic Intensity X: %d\n", Generic_magData.MagneticIntensityX);
                    OS_printf("Magnetic Intensity Y: %d\n", Generic_magData.MagneticIntensityY);
                    OS_printf("Magnetic Intensity Z: %d\n", Generic_magData.MagneticIntensityZ);
                    OS_printf("GENERIC_mag_RequestData command success\n");
                }
                else
                {
                    OS_printf("GENERIC_mag_RequestData command failed!\n");
                }
            }
            break;
        default:
            OS_printf("Invalid command format, type 'help' for more info\n");
            break;
    }
    return exit_status;
}

int main(int argc, char *argv[])
{
    int     status = OS_SUCCESS;
    char    input_buf[MAX_INPUT_BUF];
    char    input_tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE];
    int     num_input_tokens;
    int     cmd;
    char   *token_ptr;
    uint8_t run_status = OS_SUCCESS;

/* Initialize HWLIB */
#ifdef _NOS_ENGINE_LINK_
    nos_init_link();
#endif

    /* Open device specific protocols */
    Generic_magSpi.deviceString  = GENERIC_MAG_CFG_STRING;
    Generic_magSpi.handle        = GENERIC_MAG_CFG_HANDLE;
    Generic_magSpi.baudrate      = GENERIC_MAG_CFG_BAUD;
    Generic_magSpi.spi_mode      = GENERIC_MAG_CFG_SPI_MODE;
    Generic_magSpi.bits_per_word = GENERIC_MAG_CFG_BITS_PER_WORD;
    Generic_magSpi.bus           = GENERIC_MAG_CFG_BUS;
    Generic_magSpi.cs            = GENERIC_MAG_CFG_CS;
    status                       = spi_init_dev(&Generic_magSpi);
    if (status == OS_SUCCESS)
    {
        printf("SPI device %s configured with baudrate %d \n", Generic_magSpi.deviceString, Generic_magSpi.baudrate);
    }
    else
    {
        printf("SPI device %s failed to initialize! \n", Generic_magSpi.deviceString);
        run_status = OS_ERROR;
    }

    /* Main loop */
    print_help();
    while (run_status == OS_SUCCESS)
    {
        num_input_tokens = -1;
        cmd              = CMD_UNKNOWN;

        /* Read user input */
        printf(PROMPT);
        fgets(input_buf, MAX_INPUT_BUF, stdin);

        /* Tokenize line buffer */
        token_ptr = strtok(input_buf, " \t\n");
        while ((num_input_tokens < MAX_INPUT_TOKENS) && (token_ptr != NULL))
        {
            if (num_input_tokens == -1)
            {
                /* First token is command */
                cmd = get_command(token_ptr);
            }
            else
            {
                strncpy(input_tokens[num_input_tokens], token_ptr, MAX_INPUT_TOKEN_SIZE);
            }
            token_ptr = strtok(NULL, " \t\n");
            num_input_tokens++;
        }

        /* Process command if valid */
        if (num_input_tokens >= 0)
        {
            /* Process command */
            run_status = process_command(cmd, num_input_tokens, input_tokens);
        }
    }

    // Close the device
    status = spi_close_device(&Generic_magSpi);
#ifdef _NOS_ENGINE_LINK_
    nos_destroy_link();
#endif

    OS_printf("Cleanly exiting generic_mag application...\n\n");
    return 1;
}

/*
** Generic Functions
*/
int check_number_arguments(int actual, int expected)
{
    int status = OS_SUCCESS;
    if (actual != expected)
    {
        status = OS_ERROR;
        OS_printf("Invalid command format, type 'help' for more info\n");
    }
    return status;
}

void to_lower(char *str)
{
    char *ptr = str;
    while (*ptr)
    {
        *ptr = tolower((unsigned char)*ptr);
        ptr++;
    }
    return;
}
