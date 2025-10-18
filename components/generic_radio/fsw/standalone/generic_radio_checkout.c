/*******************************************************************************
** File: generic_radio_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_radio_checkout.h"

/*
** Global Variables
*/
socket_info_t                 RadioSocket;
socket_info_t                 ProxSocket;
GENERIC_RADIO_Device_HK_tlm_t RadioHK;
uint8_t                       RadioData;
uint16_t                      SCID = 0x42;

/*
** Component Functions
*/
void print_help(void)
{
    printf(PROMPT "command [args]\n"
                  "---------------------------------------------------------------------\n"
                  "help                               - Display help                    \n"
                  "exit                               - Exit app                        \n"
                  "hk                                 - Request device housekeeping     \n"
                  "  h                                - ^                               \n"
                  "proxforward                        - Perform a Proximity Forward     \n"
                  "  p                                - ^                               \n"
                  "cfg #                              - Send configuration #            \n"
                  "  c #                              - ^                               \n"
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
    else if (strcmp(lcmd, "proxforward") == 0)
    {
        status = CMD_PROX_FORWARD;
    }
    else if (strcmp(lcmd, "p") == 0)
    {
        status = CMD_PROX_FORWARD;
    }
    else if (strcmp(lcmd, "cfg") == 0)
    {
        status = CMD_CFG;
    }
    else if (strcmp(lcmd, "c") == 0)
    {
        status = CMD_CFG;
    }
    return status;
}

int process_command(int cc, int num_tokens, char tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE])
{
    int32_t  status      = OS_SUCCESS;
    int32_t  exit_status = OS_SUCCESS;
    uint32_t config;

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
                status = GENERIC_RADIO_RequestHK(&RadioSocket, &RadioHK);
                if (status == OS_SUCCESS)
                {
                    OS_printf("GENERIC_RADIO_RequestHK command success!\n");

                    OS_printf("GENERIC_RADIO_DEVICE_HK:\n");
                    OS_printf("  Device Counter:  %u\n", RadioHK.DeviceCounter);
                    OS_printf("  Device Config: 0x%07X\n", RadioHK.DeviceConfig);
                    OS_printf("  Prox Signal:     %u\n", RadioHK.ProxSignal);
                    OS_printf("\n");
                }
                else
                {
                    OS_printf("GENERIC_RADIO_RequestHK command failed!\n");
                }
            }
            break;

        case CMD_PROX_FORWARD:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = GENERIC_RADIO_ProximityForward(&RadioSocket, SCID, &RadioData, sizeof(RadioData));
                if (status == OS_SUCCESS)
                {
                    OS_printf("GENERIC_RADIO_Prox_Forward command success!\n");
                }
                else
                {
                    OS_printf("GENERIC_RADIO_PROX_FORWARD command failed!\n");
                }
            }
            break;

        case CMD_CFG:
            if (check_number_arguments(num_tokens, 1) == OS_SUCCESS)
            {
                config = atoi(tokens[0]);
                status = GENERIC_RADIO_SetConfiguration(&RadioSocket, config);
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
    ** Initialize sockets
    */
    RadioSocket.sockfd         = -1;
    RadioSocket.port_num       = GENERIC_RADIO_CFG_UDP_RADIO_TO_FSW;
    RadioSocket.ip_address     = GENERIC_RADIO_CFG_FSW_IP;
    RadioSocket.address_family = ip_ver_4;
    RadioSocket.type           = dgram;
    RadioSocket.category       = client;
    RadioSocket.block          = false;
    RadioSocket.keep_alive     = false;
    RadioSocket.created        = false;
    RadioSocket.bound          = false;
    RadioSocket.listening      = false;
    RadioSocket.connected      = false;

    status = socket_create(&RadioSocket);
    if (status != SOCKET_SUCCESS)
    {
        printf("GENERIC_RADIO: Radio interface create error %d\n", status);
        run_status = OS_ERROR;
    }
    else
    {
        printf("GENERIC_RADIO: Radio Interface %d created successfully!\n", RadioSocket.sockfd);
    }

    OS_printf("port: %d; IP: %s \n", RadioSocket.port_num, RadioSocket.ip_address);

    ProxSocket.sockfd         = -1;
    ProxSocket.port_num       = GENERIC_RADIO_CFG_UDP_PROX_TO_FSW;
    ProxSocket.ip_address     = GENERIC_RADIO_CFG_FSW_IP;
    ProxSocket.address_family = ip_ver_4;
    ProxSocket.type           = dgram;
    ProxSocket.category       = client;
    ProxSocket.block          = false;
    ProxSocket.keep_alive     = false;
    ProxSocket.created        = false;
    ProxSocket.bound          = false;
    ProxSocket.listening      = false;
    ProxSocket.connected      = false;

    status = socket_create(&ProxSocket);
    if (status != SOCKET_SUCCESS)
    {
        printf("GENERIC_RADIO: Proximity interface create error %d\n", status);
        run_status = OS_ERROR;
    }
    else
    {
        printf("GENERIC_RADIO: Proximity Interface %d created successfully!\n", ProxSocket.sockfd);
    }

    OS_printf("port: %d; IP: %s \n", ProxSocket.port_num, ProxSocket.ip_address);

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

    if (RadioSocket.sockfd != -1)
    {
        socket_close(&RadioSocket);
    }

    if (ProxSocket.sockfd != -1)
    {
        socket_close(&ProxSocket);
    }

#ifdef _NOS_ENGINE_LINK_
    nos_destroy_link();
#endif

    OS_printf("Cleanly exiting generic_radio application...\n\n");
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
