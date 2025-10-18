/*******************************************************************************
** File: generic_css_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_css_checkout.h"

/*
** Global Variables
*/
i2c_bus_info_t                Generic_CSSI2c;
GENERIC_CSS_Device_Data_tlm_t Generic_CSSData;

/*
** Component Functions
*/
void print_help(void)
{
    printf(PROMPT "command [args]\n"
                  "---------------------------------------------------------------------\n"
                  "help                               - Display help                    \n"
                  "exit                               - Exit app                        \n"
                  "css                                - Request generic_css data        \n"
                  "  c                                - ^                               \n"
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
    else if (strcmp(lcmd, "css") == 0)
    {
        status = CMD_GENERIC_CSS;
    }
    else if (strcmp(lcmd, "c") == 0)
    {
        status = CMD_GENERIC_CSS;
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

        case CMD_GENERIC_CSS:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = GENERIC_CSS_RequestData(&Generic_CSSI2c, &Generic_CSSData);
                if (status == OS_SUCCESS)
                {
                    OS_printf("GENERIC_CSS_RequestData command success\n");
                }
                else
                {
                    OS_printf("GENERIC_CSS_RequestData command failed!\n");
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
    Generic_CSSI2c.handle = GENERIC_CSS_CFG_HANDLE;
    Generic_CSSI2c.isOpen = PORT_CLOSED;
    Generic_CSSI2c.speed  = GENERIC_CSS_CFG_BAUDRATE_HZ;
    Generic_CSSI2c.addr   = GENERIC_CSS_I2C_ADDRESS;
    status                = i2c_master_init(&Generic_CSSI2c);
    if (status == OS_SUCCESS)
    {
        printf("I2C device %d configured with speed %d \n", Generic_CSSI2c.handle, Generic_CSSI2c.speed);
    }
    else
    {
        printf("I2C device %d failed to initialize! \n", Generic_CSSI2c.handle);
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
    i2c_master_close(&Generic_CSSI2c);

#ifdef _NOS_ENGINE_LINK_
    nos_destroy_link();
#endif

    OS_printf("Cleanly exiting generic_css application...\n\n");
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
