/*******************************************************************************
** File: generic_imu_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_imu_checkout.h"

/*
** Global Variables
*/
can_info_t                    Generic_IMUcan;
GENERIC_IMU_Device_HK_tlm_t   Generic_IMUHK;
GENERIC_IMU_Device_Data_tlm_t Generic_IMUData;

/*
** Component Functions
*/
void print_help(void)
{
    printf(PROMPT "command [args]\n"
                  "---------------------------------------------------------------------\n"
                  "help                               - Display help                    \n"
                  "exit                               - Exit app                        \n"
                  "noop                               - No operation command to device  \n"
                  "  n                                - ^                               \n"
                  "hk                                 - Request device housekeeping     \n"
                  "  h                                - ^                               \n"
                  "generic_imu                        - Request generic_imu data        \n"
                  "  i                                - ^                               \n"
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
    else if (strcmp(lcmd, "noop") == 0)
    {
        status = CMD_NOOP;
    }
    else if (strcmp(lcmd, "n") == 0)
    {
        status = CMD_NOOP;
    }
    else if (strcmp(lcmd, "hk") == 0)
    {
        status = CMD_HK;
    }
    else if (strcmp(lcmd, "h") == 0)
    {
        status = CMD_HK;
    }
    else if (strcmp(lcmd, "generic_imu") == 0)
    {
        status = CMD_GENERIC_IMU;
    }
    else if (strcmp(lcmd, "i") == 0)
    {
        status = CMD_GENERIC_IMU;
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

        case CMD_NOOP:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = GENERIC_IMU_CommandDevice(&Generic_IMUcan, GENERIC_IMU_DEVICE_NOOP_CMD);
                if (status == OS_SUCCESS)
                {
                    OS_printf("NOOP command success\n");
                }
                else
                {
                    OS_printf("NOOP command failed!\n");
                }
            }
            break;

        case CMD_HK:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = GENERIC_IMU_RequestHK(&Generic_IMUcan, &Generic_IMUHK);
                if (status == OS_SUCCESS)
                {
                    OS_printf("HK DeviceCounter = %u\n", Generic_IMUHK.DeviceCounter);
                    OS_printf("HK DeviceStatus = %u\n", Generic_IMUHK.DeviceStatus);
                    OS_printf("GENERIC_IMU_RequestHK command success\n");
                }
                else
                {
                    OS_printf("GENERIC_IMU_RequestHK command failed!\n");
                }
            }
            break;

        case CMD_GENERIC_IMU:
            if (check_number_arguments(num_tokens, 0) == OS_SUCCESS)
            {
                status = GENERIC_IMU_RequestData(&Generic_IMUcan, &Generic_IMUData);
                if (status == OS_SUCCESS)
                {
                    OS_printf("X_Data Linear Acceleration = %f\n", Generic_IMUData.X_Data.LinearAcc);
                    OS_printf("X_Data Angular Acceleration = %f\n", Generic_IMUData.X_Data.AngularAcc);

                    OS_printf("Y_Data Linear Acceleration = %f\n", Generic_IMUData.Y_Data.LinearAcc);
                    OS_printf("Y_Data Angular Acceleration = %f\n", Generic_IMUData.Y_Data.AngularAcc);

                    OS_printf("Z_Data Linear Acceleration = %f\n", Generic_IMUData.Z_Data.LinearAcc);
                    OS_printf("Z_Data Angular Acceleration = %f\n", Generic_IMUData.Z_Data.AngularAcc);

                    OS_printf("GENERIC_IMU_RequestData command success\n");
                }
                else
                {
                    OS_printf("GENERIC_IMU_RequestData command failed!\n");
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
    Generic_IMUcan.handle              = GENERIC_IMU_CFG_HANDLE;
    Generic_IMUcan.isUp                = CAN_INTERFACE_DOWN;
    Generic_IMUcan.loopback            = false;
    Generic_IMUcan.listenOnly          = false;
    Generic_IMUcan.tripleSampling      = false;
    Generic_IMUcan.oneShot             = false;
    Generic_IMUcan.berrReporting       = false;
    Generic_IMUcan.fd                  = false;
    Generic_IMUcan.presumeAck          = false;
    Generic_IMUcan.bitrate             = GENERIC_IMU_CFG_CAN_BITRATE;
    Generic_IMUcan.second_timeout      = GENERIC_IMU_CFG_CAN_TIMEOUT;
    Generic_IMUcan.microsecond_timeout = GENERIC_IMU_CFG_CAN_MS_TIMEOUT;
    Generic_IMUcan.xfer_us_delay       = GENERIC_IMU_CFG_CAN_XFER_US;

    status = can_init_dev(&Generic_IMUcan);

    if (status == OS_SUCCESS)
    {
        OS_printf("GENERIC_IMU: CAN port initialization success %d", status);
    }
    else
    {
        OS_printf("GENERIC_IMU: CAN port initialization success %d", status);
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
    can_close_device(&Generic_IMUcan);

#ifdef _NOS_ENGINE_LINK_
    nos_destroy_link();
#endif

    OS_printf("Cleanly exiting generic_imu application...\n\n");
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
