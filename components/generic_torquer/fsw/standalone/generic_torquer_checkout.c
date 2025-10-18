/*******************************************************************************
** File: generic_torquer_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_torquer_checkout.h"

/*
** Global Variables
*/
trq_info_t                   trqDevice;
GENERIC_TORQUER_Device_tlm_t trqHk;

/*
** Component Functions
*/
void print_help(void)
{
    printf(PROMPT "command [args]\n"
                  "---------------------------------------------------------------------\n"
                  "help                               - Display help                    \n"
                  "exit                               - Exit app                        \n"
                  "hk                                 - Print current housekeeping      \n"
                  "  h                                - ^                               \n"
                  "torque percent# direction#         - Torque percent (0-100) and direction (0-1) \n"
                  "  t # #                            - ^                               \n"
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
    else if (strcmp(lcmd, "hk") == 0)
    {
        status = CMD_HK;
    }
    else if (strcmp(lcmd, "h") == 0)
    {
        status = CMD_HK;
    }
    else if (strcmp(lcmd, "torque") == 0)
    {
        status = CMD_TORQUE;
    }
    else if (strcmp(lcmd, "t") == 0)
    {
        status = CMD_TORQUE;
    }
    return status;
}

int process_command(int cc, int num_tokens, char tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE])
{
    int32_t  status      = OS_SUCCESS;
    int32_t  exit_status = OS_SUCCESS;
    uint32_t req_percent, req_direction;

    /* Process command */
    switch (cc)
    {
        case CMD_HELP:
            print_help();
            break;

        case CMD_EXIT:
            exit_status = OS_ERROR;
            break;

        case CMD_HK:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                OS_printf("trqHk.Direction = %d \n", trqHk.Direction);
                OS_printf("trqHk.PercentOn = %d \n", trqHk.PercentOn);
            }
            break;

        case CMD_TORQUE:
            if (check_number_arguments(num_tokens, 2) == OS_SUCCESS)
            {
                req_percent   = atoi(tokens[0]);
                req_direction = atoi(tokens[1]);

                // TODO - add error checking to the above

                status = GENERIC_TORQUER_Config(&trqHk, &trqDevice, req_percent, req_direction);
                if (status == OS_SUCCESS)
                {
                    OS_printf("GENERIC_TORQUER_Config command success\n");
                }
                else
                {
                    OS_printf("GENERIC_TORQUER_Config command failed!\n");
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
    trqHk.Direction = 0;
    trqHk.PercentOn = 0;

    trqDevice.trq_num            = 0;
    trqDevice.timer_period_ns    = GENERIC_TORQUER_CFG_PERIOD;
    trqDevice.timerfd            = 0;
    trqDevice.direction_pin_fd   = 0;
    trqDevice.timer_high_ns      = 0;
    trqDevice.positive_direction = false;
    trqDevice.enabled            = false;

    status = trq_init(&trqDevice);
    if (status == OS_SUCCESS)
    {
        printf("Torquer initialized successfully \n");
    }
    else
    {
        printf("Torquer device failed to initialize with error %d!\n", status);
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
    trq_close(&trqDevice);

#ifdef _NOS_ENGINE_LINK_
    nos_destroy_link();
#endif

    OS_printf("Cleanly exiting generic_torquer application...\n\n");
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
