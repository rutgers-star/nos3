/*******************************************************************************
** File: novatel_oem615_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "novatel_oem615_checkout.h"

/*
** Global Variables
*/
uart_info_t                      Novatel_oem615Uart; /* Hardware protocol definition */
NOVATEL_OEM615_Device_Data_tlm_t Novatel_oem615Data;

/*
** Component Functions
*/
void print_help(void)
{
    printf(PROMPT "command [args]\n"
                  "---------------------------------------------------------------------\n"
                  "help                               - Display help                    \n"
                  "exit                               - Exit app                        \n"
                  "data                               - Request novatel_oem615 data     \n"
                  "d                                  - ^                               \n"
                  "serialconfig #                     - Serial configuration #          \n"
                  "s #                                - ^                               \n"
                  "log # #                            - Log novatel_oem615 data         \n"
                  "l                                  - ^                               \n"
                  "unlog #                            - Unlog novatel_oem615 data       \n"
                  "u                                  - ^                               \n"
                  "unlog_all                          - Unlog all novatel_oem615 data   \n"
                  "ua                                 - ^                               \n"
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
    else if (strcmp(lcmd, "data") == 0)
    {
        status = CMD_NOVATEL_OEM615;
    }
    else if (strcmp(lcmd, "d") == 0)
    {
        status = CMD_NOVATEL_OEM615;
    }
    else if (strcmp(lcmd, "serialconfig") == 0)
    {
        status = CMD_CFG;
    }
    else if (strcmp(lcmd, "s") == 0)
    {
        status = CMD_CFG;
    }
    else if (strcmp(lcmd, "log") == 0)
    {
        status = CMD_LOG;
    }
    else if (strcmp(lcmd, "l") == 0)
    {
        status = CMD_LOG;
    }
    else if (strcmp(lcmd, "unlog") == 0)
    {
        status = CMD_UNLOG;
    }
    else if (strcmp(lcmd, "u") == 0)
    {
        status = CMD_UNLOG;
    }
    else if (strcmp(lcmd, "unlog all") == 0)
    {
        status = CMD_UNLOG_ALL;
    }
    else if (strcmp(lcmd, "ua") == 0)
    {
        status = CMD_UNLOG_ALL;
    }
    return status;
}

int process_command(int cc, int num_tokens, char tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE])
{
    int32_t  status      = OS_SUCCESS;
    int32_t  exit_status = OS_SUCCESS;
    uint32_t config;
    int8_t   log_type;
    int8_t   period;

    /* Process command */
    switch (cc)
    {
        case CMD_HELP:
            print_help();
            break;

        case CMD_EXIT:
            exit_status = OS_ERROR;
            break;

        case CMD_NOVATEL_OEM615:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = NOVATEL_OEM615_ChildProcessReadData(&Novatel_oem615Uart, &Novatel_oem615Data);

                if (status == OS_SUCCESS)
                {
                    OS_printf("NOVATEL_OEM615_RequestData command success\n");
                }
                else
                {
                    OS_printf("NOVATEL_OEM615_RequestData command failed!\n");
                }
            }
            break;

        case CMD_CFG:
            if (check_number_arguments(num_tokens, 1) == OS_SUCCESS)
            {
                config = atoi(tokens[0]);

                status = NOVATEL_OEM615_CommandDevice(&Novatel_oem615Uart, 7, config, 0);

                if (status == OS_SUCCESS)
                {
                    OS_printf("Configuration command success with value %u\n", config);
                }
                else
                {
                    OS_printf("Configuration command failed!\n");
                }
            }
            break;

        case CMD_LOG:
            if (check_number_arguments(num_tokens, 2) == OS_SUCCESS)
            {
                log_type = atoi(tokens[0]);
                period   = atoi(tokens[1]);
                status   = NOVATEL_OEM615_CommandDevice(&Novatel_oem615Uart, 4, log_type, period);
                if (status == OS_SUCCESS)
                {
                    OS_printf("LOG command success!\n");
                }
                else
                {
                    OS_printf("LOG command failed!\n");
                }
            }
            break;

        case CMD_UNLOG:
            if (check_number_arguments(num_tokens, 1) == OS_SUCCESS)
            {
                log_type = atoi(tokens[0]);
                status   = NOVATEL_OEM615_CommandDevice(&Novatel_oem615Uart, 5, log_type, 0);
                if (status == OS_SUCCESS)
                {
                    OS_printf("UNLOG command success!\n");
                }
                else
                {
                    OS_printf("UNLOG command failed!\n");
                }
            }
            break;

        case CMD_UNLOG_ALL:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = NOVATEL_OEM615_CommandDevice(&Novatel_oem615Uart, 6, 0, 0);
                if (status == OS_SUCCESS)
                {
                    OS_printf("UNLOG_ALL command success!\n");
                }
                else
                {
                    OS_printf("UNLOG_ALL command failed!\n");
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
    Novatel_oem615Uart.deviceString  = NOVATEL_OEM615_CFG_STRING;
    Novatel_oem615Uart.handle        = NOVATEL_OEM615_CFG_HANDLE;
    Novatel_oem615Uart.isOpen        = PORT_CLOSED;
    Novatel_oem615Uart.baud          = NOVATEL_OEM615_CFG_BAUDRATE_HZ;
    Novatel_oem615Uart.access_option = uart_access_flag_RDWR;

    status = uart_init_port(&Novatel_oem615Uart);
    if (status == OS_SUCCESS)
    {
        OS_printf("UART device %s configured with baudrate %d \n", Novatel_oem615Uart.deviceString,
                  Novatel_oem615Uart.baud);
    }
    else
    {
        OS_printf("UART device %s failed to initialize! \n", Novatel_oem615Uart.deviceString);
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
    uart_close_port(&Novatel_oem615Uart);

#ifdef _NOS_ENGINE_LINK_
    nos_destroy_link();
#endif

    OS_printf("Cleanly exiting novatel_oem615 application...\n\n");
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