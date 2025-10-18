/*******************************************************************************
** File: generic_fss_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_fss_checkout.h"

/*
** Global Variables
*/
spi_info_t                    FssSpi;
GENERIC_FSS_Device_Data_tlm_t FSSData;

/*
** Component Functions
*/
void fss_print_help(void)
{
    printf(PROMPT "command [args]\n"
                  "---------------------------------------------------------------------\n"
                  "help                               - Display help                    \n"
                  "exit                               - Exit app                        \n"
                  "fss                                - Request generic_fss data        \n"
                  "  f                                - ^                               \n"
                  "\n");
}

int fss_get_command(const char *str)
{
    int  status = CMD_UNKNOWN;
    char lcmd[MAX_INPUT_TOKEN_SIZE];
    strncpy(lcmd, str, MAX_INPUT_TOKEN_SIZE);

    /* Convert command to lower case */
    fss_to_lower(lcmd);

    if (strcmp(lcmd, "help") == 0)
    {
        status = CMD_HELP;
    }
    else if (strcmp(lcmd, "exit") == 0)
    {
        status = CMD_EXIT;
    }
    else if (strcmp(lcmd, "fss") == 0)
    {
        status = CMD_GENERIC_FSS;
    }
    else if (strcmp(lcmd, "f") == 0)
    {
        status = CMD_GENERIC_FSS;
    }
    return status;
}

int fss_process_command(int cc, int num_tokens, char tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE])
{
    int32_t status      = OS_SUCCESS;
    int32_t exit_status = OS_SUCCESS;

    /* Process command */
    switch (cc)
    {
        case CMD_HELP:
            fss_print_help();
            break;

        case CMD_EXIT:
            exit_status = OS_ERROR;
            break;

        case CMD_GENERIC_FSS:
            if (fss_check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = GENERIC_FSS_RequestData(&FssSpi, &FSSData);
                if (status == OS_SUCCESS)
                {
                    OS_printf("GENERIC_FSS_RequestData command success.\n");
                }
                else
                {
                    OS_printf("GENERIC_FSS_RequestData command failed!\n");
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

    /*
    ** Initialize hardware interface data
    */
    FssSpi.deviceString  = GENERIC_FSS_CFG_STRING;
    FssSpi.handle        = GENERIC_FSS_CFG_HANDLE;
    FssSpi.baudrate      = GENERIC_FSS_CFG_BAUD;
    FssSpi.spi_mode      = GENERIC_FSS_CFG_SPI_MODE;
    FssSpi.bits_per_word = GENERIC_FSS_CFG_BITS_PER_WORD;
    FssSpi.bus           = GENERIC_FSS_CFG_BUS;
    FssSpi.cs            = GENERIC_FSS_CFG_CS;

    /* Open device specific protocols */
    status = spi_init_dev(&FssSpi);
    if (status == OS_SUCCESS)
    {
        printf("SPI device %s configured with baudrate %d \n", FssSpi.deviceString, FssSpi.baudrate);
    }
    else
    {
        printf("SPI device %s failed to initialize! \n", FssSpi.deviceString);
        run_status = OS_ERROR;
    }

    /* Main loop */
    fss_print_help();
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
                cmd = fss_get_command(token_ptr);
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
            run_status = fss_process_command(cmd, num_input_tokens, input_tokens);
        }
    }

    // Close the device
    spi_close_device(&FssSpi);

#ifdef _NOS_ENGINE_LINK_
    nos_destroy_link();
#endif

    OS_printf("Cleanly exiting generic_fss application...\n\n");
    return 1;
}

/*
** Generic Functions
*/
int fss_check_number_arguments(int actual, int expected)
{
    int status = OS_SUCCESS;
    if (actual != expected)
    {
        status = OS_ERROR;
        OS_printf("Invalid command format, type 'help' for more info\n");
    }
    return status;
}

void fss_to_lower(char *str)
{
    char *ptr = str;
    while (*ptr)
    {
        *ptr = tolower((unsigned char)*ptr);
        ptr++;
    }
    return;
}
