/*******************************************************************************
** File: novatel_oem615_device.c
**
** Purpose:
**   This file contains the source code for the NOVATEL_OEM615 device.
**
*******************************************************************************/

/*
** Include Files
*/
#include "novatel_oem615_device.h"

static char *saveptr;

int32_t NOVATEL_OEM615_CommandDevice(uart_info_t *uart_device, uint8_t cmd_code, int8_t log_type, int8_t period_option)
{
    int32_t  status         = OS_SUCCESS;
    int32_t  command_length = 0;
    uint8_t *write_data;
    char    *log_type_str;
    char    *period_option_str;

    // log
    if (cmd_code == 4)
    {
        command_length = sizeof(NOVATEL_OEM615_LOG) + sizeof(NOVATEL_OEM615_PORT);
        switch (log_type)
        {
            case 0:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_BESTXYZA);
                log_type_str = NOVATEL_OEM615_LOGTYPE_BESTXYZA;
                break;
            case 1:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_GPGGA);
                log_type_str = NOVATEL_OEM615_LOGTYPE_GPGGA;
                break;
            case 2:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_RANGECMPA);
                log_type_str = NOVATEL_OEM615_LOGTYPE_RANGECMPA;
                break;
            case 3:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_BESTXYZB);
                log_type_str = NOVATEL_OEM615_LOGTYPE_BESTXYZB;
                break;
            case 4:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_RANGECMPB);
                log_type_str = NOVATEL_OEM615_LOGTYPE_RANGECMPB;
                break;
            default:
                status = OS_ERROR;
                break;
        }
        switch (period_option)
        {
            case 0:
                command_length += sizeof(NOVATEL_OEM615_PERIOD_OPTION_0);
                period_option_str = NOVATEL_OEM615_PERIOD_OPTION_0;
                break;
            case 1:
                command_length += sizeof(NOVATEL_OEM615_PERIOD_OPTION_1);
                period_option_str = NOVATEL_OEM615_PERIOD_OPTION_1;
                break;
            case 2:
                command_length += sizeof(NOVATEL_OEM615_PERIOD_OPTION_2);
                period_option_str = NOVATEL_OEM615_PERIOD_OPTION_2;
                break;
            case 3:
                command_length += sizeof(NOVATEL_OEM615_PERIOD_OPTION_3);
                period_option_str = NOVATEL_OEM615_PERIOD_OPTION_3;
                break;
            case 4:
                command_length += sizeof(NOVATEL_OEM615_PERIOD_OPTION_4);
                period_option_str = NOVATEL_OEM615_PERIOD_OPTION_4;
                break;
            case 5:
                command_length += sizeof(NOVATEL_OEM615_PERIOD_OPTION_5);
                period_option_str = NOVATEL_OEM615_PERIOD_OPTION_5;
                break;
            default:
                status = OS_ERROR;
                break;
        }
        if (status == OS_SUCCESS)
        {
            write_data = (uint8_t *)calloc(command_length, sizeof(uint8_t));
            strcpy((char *)write_data, NOVATEL_OEM615_LOG);
            strcat((char *)write_data, NOVATEL_OEM615_PORT);
            strcat((char *)write_data, log_type_str);
            strcat((char *)write_data, period_option_str);
        }
    }
    // unlog
    else if (cmd_code == 5)
    {
        command_length = sizeof(NOVATEL_OEM615_UNLOG) + sizeof(NOVATEL_OEM615_PORT);
        switch (log_type)
        {
            case 0:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_BESTXYZA);
                log_type_str = NOVATEL_OEM615_LOGTYPE_BESTXYZA;
                break;
            case 1:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_GPGGA);
                log_type_str = NOVATEL_OEM615_LOGTYPE_GPGGA;
                break;
            case 2:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_RANGECMPA);
                log_type_str = NOVATEL_OEM615_LOGTYPE_RANGECMPA;
                break;
            case 3:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_BESTXYZB);
                log_type_str = NOVATEL_OEM615_LOGTYPE_BESTXYZB;
                break;
            case 4:
                command_length += sizeof(NOVATEL_OEM615_LOGTYPE_RANGECMPB);
                log_type_str = NOVATEL_OEM615_LOGTYPE_RANGECMPB;
                break;
            default:
                status = OS_ERROR;
                break;
        }
        if (status == OS_SUCCESS)
        {
            write_data = (uint8_t *)calloc(command_length, sizeof(uint8_t));
            strcpy((char *)write_data, NOVATEL_OEM615_UNLOG);
            strcat((char *)write_data, NOVATEL_OEM615_PORT);
            strcat((char *)write_data, log_type_str);
        }
    }
    // unlogall
    else if (cmd_code == 6)
    {
        command_length = sizeof(NOVATEL_OEM615_UNLOGALL);
        write_data     = (uint8_t *)calloc(command_length, sizeof(uint8_t));
        strcpy((char *)write_data, NOVATEL_OEM615_UNLOGALL);
    }
    // serialconfig
    else if (cmd_code == 7)
    {
        command_length = sizeof(NOVATEL_OEM615_SERIALCONFIG);
        write_data     = (uint8_t *)calloc(command_length, sizeof(uint8_t));
        strcpy((char *)write_data, NOVATEL_OEM615_SERIALCONFIG);
    }

    // Flush any prior data
    if (status == OS_SUCCESS)
    {
        status = uart_flush(uart_device);
        if (status == OS_SUCCESS)
        {
            // Write data
            uart_write_port(uart_device, write_data, command_length);
            free(write_data);
        } // uart_flush
        else
        {
#ifdef NOVATEL_OEM615_CFG_DEBUG
            OS_printf("NOVATEL_OEM615_CommandDevice - uart_flush returned error with status = %d! No uart_write_port "
                      "performed.",
                      status);
#endif
            status = OS_ERROR;
        }
    }
    return status;
}

/*
** Request data command
*/
int32_t NOVATEL_OEM615_RequestData(uart_info_t *uart_device, NOVATEL_OEM615_Device_Data_tlm_t *data)
{
    int32_t status          = OS_SUCCESS;
    int32_t bytes           = 0;
    int32_t bytes_available = 0;
    char   *token;

    if (status == OS_SUCCESS)
    {
        /* check how many bytes are waiting on the uart */
        bytes_available = uart_bytes_available(uart_device);
        if (bytes_available > 0)
        {
            uint8_t *temp_read_data = (uint8_t *)calloc(bytes_available, sizeof(uint8_t));
            /* Read all existing data on uart port */
            bytes = uart_read_port(uart_device, temp_read_data, bytes_available);
            if (bytes != bytes_available)
            {
#ifdef NOVATEL_OEM615_CFG_DEBUG
                OS_printf("  NOVATEL_OEM615_RequestData: Bytes read != to requested! \n");
#endif
                status = OS_ERROR;
                free(temp_read_data);
            }
            else
            {
/* search uart data for token signifying start of bestxyza gps data packet */
#ifdef NOVATEL_OEM615_CFG_DEBUG
                OS_printf(" ALL UART BYTES READ FROM BUFFER: \n");
                for (int i = 0; i < bytes; i++)
                {
                    OS_printf("%02x", temp_read_data[i]);
                }
                OS_printf(" DONE PRINTING ALL UART BYTES READ FROM BUFFER \n");
#endif
                token = strtok_r((char *)temp_read_data, ",", &saveptr);
                if ((token != NULL) && (strncmp(token, "#BESTXYZA", 9) == 0))
                {
#ifdef NOVATEL_OEM615_CFG_DEBUG
                    OS_printf(" DATA TOKEN FOUND = %s\n", token);
#endif

                    NOVATEL_OEM615_ParseBestXYZA(data);
                }
                else
                {
#ifdef NOVATEL_OEM615_CFG_DEBUG
                    OS_printf("  NOVATEL_OEM615_RequestData: No #BESTXYZA token found when reading uart port! \n");
#endif
                    status = OS_ERROR;
                    free(temp_read_data);
                }
            }
        }
        else
        {
#ifdef NOVATEL_OEM615_CFG_DEBUG
            OS_printf("  NOVATEL_OEM615_RequestData: No data available when attempting to read from uart port! \n");
#endif
            status = OS_ERROR;
        }
    }
    else
    {
#ifdef NOVATEL_OEM615_CFG_DEBUG
        OS_printf("  NOVATEL_OEM615_RequestData: CommandDevice returned error with status = %d! \n", status);
#endif
        status = OS_ERROR;
    }
    return status;
}

/*
** Request data command for child task
*/
int32_t NOVATEL_OEM615_ChildProcessReadData(uart_info_t *uart_device, NOVATEL_OEM615_Device_Data_tlm_t *data)
{
    int32_t status          = OS_SUCCESS;
    int32_t bytes           = 0;
    int32_t bytes_available = 0;
    char   *token;

    /* check how many bytes are waiting on the uart */
    bytes_available = uart_bytes_available(uart_device);
    if (bytes_available > 0)
    {
        uint8_t *temp_read_data = (uint8_t *)calloc(bytes_available, sizeof(uint8_t));
        /* Read all existing data on uart port */
        bytes = uart_read_port(uart_device, temp_read_data, bytes_available);
        if (bytes != bytes_available)
        {
#ifdef NOVATEL_OEM615_CFG_DEBUG
            OS_printf("  NOVATEL_OEM615_ChildProcessReadData: Bytes read != to requested! \n");
#endif
            status = OS_ERROR;
            free(temp_read_data);
        }
        else
        {
/* search uart data for token signifying start of bestxyza gps data packet */
#ifdef NOVATEL_OEM615_CFG_DEBUG
            OS_printf(" ALL UART BYTES READ FROM BUFFER: \n");
            for (int i = 0; i < bytes; i++)
            {
                OS_printf("%02x", temp_read_data[i]);
            }
            OS_printf(" DONE PRINTING ALL UART BYTES READ FROM BUFFER \n");
#endif
            token = strtok_r((char *)temp_read_data, ",", &saveptr);
            if ((token != NULL) && (strncmp(token, "#BESTXYZA", 9) == 0))
            {
#ifdef NOVATEL_OEM615_CFG_DEBUG
                OS_printf(" DATA TOKEN FOUND = %s\n", token);
#endif

                NOVATEL_OEM615_ParseBestXYZA(data);
            }
            else if ((token != NULL) && (strncmp(token, "$GPGGA", 6) == 0))
            {
#ifdef NOVATEL_OEM615_CFG_DEBUG
                OS_printf(" DATA TOKEN FOUND = %s\n", token);
#endif

                NOVATEL_OEM615_ParseBestGPGGA(data);
            }
            else
            {
#ifdef NOVATEL_OEM615_CFG_DEBUG
                OS_printf("  NOVATEL_OEM615_ChildProcessReadData: No #BESTXYZA token found when reading uart port! \n");
#endif
                status = OS_ERROR;
                free(temp_read_data);
            }
        }
    }
    else
    {
#ifdef NOVATEL_OEM615_CFG_DEBUG
        OS_printf(
            "  NOVATEL_OEM615_ChildProcessReadData: No data available when attempting to read from uart port! \n");
#endif
        status = OS_ERROR;
    }
    return status;
}

/************************************************************************
** Parse NovAtel OEM615 BESTXYZA log data
*************************************************************************/
// Reference:  Section 1.1.1, p. 24, OEM6 Family Firmware Reference Manual, OM-20000129, Rev 8, January 2015 (file
// om-20000129.pdf) Reference:  Section 3.2.17, pp. 420-422, OEM6 Family Firmware Reference Manual, OM-20000129, Rev 8,
// January 2015 (file om-20000129.pdf)
void NOVATEL_OEM615_ParseBestXYZA(NOVATEL_OEM615_Device_Data_tlm_t *device_data_struct)
{
    device_data_struct->Weeks           = 0;
    device_data_struct->SecondsIntoWeek = 0;
    device_data_struct->Fractions       = 0.0;
    device_data_struct->ECEFX           = 0.0;
    device_data_struct->ECEFY           = 0.0;
    device_data_struct->ECEFZ           = 0.0;
    device_data_struct->VelX            = 0.0;
    device_data_struct->VelY            = 0.0;
    device_data_struct->VelZ            = 0.0;
    char *token;

    token = strtok_r(NULL, ",; ", &saveptr); // Port
    token = strtok_r(NULL, ",; ", &saveptr); // Sequence #
    token = strtok_r(NULL, ",; ", &saveptr); // % Idle Time
    token = strtok_r(NULL, ",; ", &saveptr); // Time Status
    token = strtok_r(NULL, ",; ", &saveptr); // Week
    if (token != NULL)
        device_data_struct->Weeks = atol(token);
    token = strtok_r(NULL, ",; ", &saveptr); // Seconds
    if (token != NULL)
    {
        device_data_struct->Fractions       = atof(token);
        device_data_struct->SecondsIntoWeek = (uint32_t)device_data_struct->Fractions;
        device_data_struct->Fractions -= device_data_struct->SecondsIntoWeek;
    }
    token = strtok_r(NULL, ",; ", &saveptr); // Receiver Status
    token = strtok_r(NULL, ",; ", &saveptr); // Reserved
    token = strtok_r(NULL, ",; ", &saveptr); // Receiver s/w Version

    // Now the data
    token = strtok_r(NULL, ",; ", &saveptr); // P-sol status
    token = strtok_r(NULL, ",; ", &saveptr); // pos type
    token = strtok_r(NULL, ",; ", &saveptr); // P-X (m)
    if (token != NULL)
        device_data_struct->ECEFX = atof(token);
    token = strtok_r(NULL, ",; ", &saveptr); // P-Y (m)
    if (token != NULL)
        device_data_struct->ECEFY = atof(token);
    token = strtok_r(NULL, ",; ", &saveptr); // P-Z (m)
    if (token != NULL)
        device_data_struct->ECEFZ = atof(token);
    token = strtok_r(NULL, ",; ", &saveptr); // P-X sigma
    token = strtok_r(NULL, ",; ", &saveptr); // P-Y sigma
    token = strtok_r(NULL, ",; ", &saveptr); // P-Z sigma
    token = strtok_r(NULL, ",; ", &saveptr); // V-sol status
    token = strtok_r(NULL, ",; ", &saveptr); // vel type
    token = strtok_r(NULL, ",; ", &saveptr); // V-X (m/s
    if (token != NULL)
        device_data_struct->VelX = atof(token);
    token = strtok_r(NULL, ",; ", &saveptr); // V-Y (m/s)
    if (token != NULL)
        device_data_struct->VelY = atof(token);
    token = strtok_r(NULL, ",; ", &saveptr); // V-Z (m/s)
    if (token != NULL)
        device_data_struct->VelZ = atof(token);

#ifdef NOVATEL_OEM615_CFG_DEBUG
    OS_printf("  Weeks = %d\n", device_data_struct->Weeks);
    OS_printf("  SecondsIntoWeek = %d\n", device_data_struct->SecondsIntoWeek);
    OS_printf("  Fractions = %f\n", device_data_struct->Fractions);
    OS_printf("  ECEFX = %f\n", device_data_struct->ECEFX);
    OS_printf("  ECEFY = %f\n", device_data_struct->ECEFY);
    OS_printf("  ECEFZ = %f\n", device_data_struct->ECEFZ);
    OS_printf("  VelX = %f\n", device_data_struct->VelX);
    OS_printf("  VelY = %f\n", device_data_struct->VelY);
    OS_printf("  VelZ = %f\n", device_data_struct->VelZ);
#endif

} /* NAV_ParseOEM615Bestxyza */

/************************************************************************
** Parse NovAtel OEM615 BESTGPGGA log data
*************************************************************************/
// Reference:  Section 1.1.1, p. 24, OEM6 Family Firmware Reference Manual, OM-20000129, Rev 8, January 2015 (file
// om-20000129.pdf) Reference:  Section 3.2.17, pp. 420-422, OEM6 Family Firmware Reference Manual, OM-20000129, Rev 8,
// January 2015 (file om-20000129.pdf)
void NOVATEL_OEM615_ParseBestGPGGA(NOVATEL_OEM615_Device_Data_tlm_t *device_data_struct)
{
    device_data_struct->lat = 0;
    device_data_struct->lon = 0;
    device_data_struct->alt = 0;

    char        *token;
    GPGGA_Data_t data;

    // Parse each field
    int field = 0;
    while ((token = strtok_r(NULL, ",", &saveptr)) != NULL)
    {
        field++;
        switch (field)
        {
            case 1: // UTC Time
                strncpy(data.utc_time, token, sizeof(data.utc_time) - 1);
                data.utc_time[sizeof(data.utc_time) - 1] = '\0';
                break;
            case 2: // Latitude
                data.latitude = atof(token);
                break;
            case 3: // N/S Indicator
                data.lat_direction = token[0];
                break;
            case 4: // Longitude
                data.longitude = atof(token);
                break;
            case 5: // E/W Indicator
                data.lon_direction = token[0];
                break;
            case 6: // Position Fix Indicator
                data.fix_quality = atoi(token);
                break;
            case 7: // Satellites Used
                data.num_satellites = atoi(token);
                break;
            case 8: // HDOP
                data.hdop = atof(token);
                break;
            case 9: // MSL Altitude
                data.altitude = atof(token);
                break;
            case 10: // Units (Altitude)
                data.altitude_units = token[0];
                break;
            case 11: // Geoid Separation
                data.geoid_separation = atof(token);
                break;
            case 12: // Units (Geoid Separation)
                data.geoid_units = token[0];
                break;
            case 13: // Age of Diff. Corr.
                strncpy(data.dgps_age, token, sizeof(data.dgps_age) - 1);
                data.dgps_age[sizeof(data.dgps_age) - 1] = '\0';
                break;
            case 14: // Diff. Ref. Station ID
                strncpy(data.dgps_station_id, token, sizeof(data.dgps_station_id) - 1);
                data.dgps_station_id[sizeof(data.dgps_station_id) - 1] = '\0';
                break;
            default:
                break;
        }
    }

    // Convert latitude and longitude from NMEA format to decimal degrees
    // NMEA latitude format is ddmm.mmmm
    int   lat_deg = (int)(data.latitude / 100);
    float lat_min = data.latitude - (lat_deg * 100);
    data.latitude = lat_deg + (lat_min / 60.0);
    if (data.lat_direction == 'S')
    {
        data.latitude = -data.latitude;
    }

    // NMEA longitude format is dddmm.mmmm
    int   lon_deg  = (int)(data.longitude / 100);
    float lon_min  = data.longitude - (lon_deg * 100);
    data.longitude = lon_deg + (lon_min / 60.0);
    if (data.lon_direction == 'W')
    {
        data.longitude = -data.longitude;
    }

    // Update Device Data struct with latest
    device_data_struct->lat = data.latitude;
    device_data_struct->lon = data.longitude;
    device_data_struct->alt = data.altitude;
}