/*******************************************************************************
** File: generic_thruster_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_thruster_checkout.h"

/*
** Global Variables
*/
uart_info_t ThrusterUart;

/*
** Component Functions
*/
void print_help(void)
{
    printf(PROMPT "command [args]\n"
                  "---------------------------------------------------------------------\n"
                  "help                               - Display help                    \n"
                  "exit                               - Exit app                        \n"
                  "percentage # #                     - Set thruster # to percentage #  \n"
                  "  p # #                            - ^                               \n"
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
    else if (strcmp(lcmd, "percentage") == 0)
    {
        status = CMD_PERCENTAGE;
    }
    else if (strcmp(lcmd, "p") == 0)
    {
        status = CMD_PERCENTAGE;
    }
    return status;
}

int process_command(int cc, int num_tokens, char tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE])
{
    int32_t status      = OS_SUCCESS;
    int32_t exit_status = OS_SUCCESS;
    uint8_t thruster_number;
    uint8_t percentage;

    /* Process command */
    switch (cc)
    {
        case CMD_HELP:
            print_help();
            break;

        case CMD_EXIT:
            exit_status = OS_ERROR;
            break;

        case CMD_PERCENTAGE:
            if (check_number_arguments(num_tokens, 2) == OS_SUCCESS)
            {
                thruster_number = atoi(tokens[0]);
                percentage      = atoi(tokens[1]);
                status          = GENERIC_THRUSTER_SetPercentage(&ThrusterUart, thruster_number, percentage,
                                                                 GENERIC_THRUSTER_DEVICE_CMD_SIZE);
                if (status == OS_SUCCESS)
                {
                    OS_printf("Thruster %d command success with value %u\n", thruster_number, percentage);
                }
                else
                {
                    OS_printf("Configuration command failed!\n");
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
    ThrusterUart.deviceString = GENERIC_THRUSTER_CFG_STRING;
    ThrusterUart.handle       = GENERIC_THRUSTER_CFG_HANDLE;
    ThrusterUart.isOpen       = PORT_CLOSED;
    ThrusterUart.baud         = GENERIC_THRUSTER_CFG_BAUDRATE_HZ;
    status                    = uart_init_port(&ThrusterUart);
    if (status == OS_SUCCESS)
    {
        printf("UART device %s configured with baudrate %d \n", ThrusterUart.deviceString, ThrusterUart.baud);
    }
    else
    {
        printf("UART device %s failed to initialize! \n", ThrusterUart.deviceString);
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
    uart_close_port(&ThrusterUart);

#ifdef _NOS_ENGINE_LINK_
    nos_destroy_link();
#endif

    OS_printf("Cleanly exiting generic_thruster application...\n\n");
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
