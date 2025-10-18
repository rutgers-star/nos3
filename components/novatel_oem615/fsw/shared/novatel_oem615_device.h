/*******************************************************************************
** File: novatel_oem615_device.h
**
** Purpose:
**   This is the header file for the NOVATEL_OEM615 device.
**
*******************************************************************************/
#ifndef _NOVATEL_OEM615_DEVICE_H_
#define _NOVATEL_OEM615_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "hwlib.h"
#include "novatel_oem615_platform_cfg.h"

/*
** Type definitions
*/
#define NOVATEL_OEM615_DEVICE_HDR   0xDEAD
#define NOVATEL_OEM615_DEVICE_HDR_0 0xDE
#define NOVATEL_OEM615_DEVICE_HDR_1 0xAD

#define NOVATEL_OEM615_DEVICE_NOOP_CMD     0x00
#define NOVATEL_OEM615_DEVICE_REQ_HK_CMD   0x01
#define NOVATEL_OEM615_DEVICE_REQ_DATA_CMD 0x02

#define NOVATEL_OEM615_DEVICE_TRAILER   0xBEEF
#define NOVATEL_OEM615_DEVICE_TRAILER_0 0xBE
#define NOVATEL_OEM615_DEVICE_TRAILER_1 0xEF

#define NOVATEL_OEM615_DEVICE_HDR_TRL_LEN 4
#define NOVATEL_OEM615_DEVICE_CMD_SIZE    9

#define NOVATEL_OEM615_LOG               "LOG "
#define NOVATEL_OEM615_UNLOG             "UNLOG "
#define NOVATEL_OEM615_UNLOGALL          "UNLOGALL"
#define NOVATEL_OEM615_SERIALCONFIG      "SERIALCONFIG"
#define NOVATEL_OEM615_LOGTYPE_BESTXYZA  "BESTXYZA "
#define NOVATEL_OEM615_LOGTYPE_GPGGA     "GPGGA "
#define NOVATEL_OEM615_LOGTYPE_RANGECMPA "RANGECMPA "
#define NOVATEL_OEM615_LOGTYPE_BESTXYZB  "BESTXYZB "
#define NOVATEL_OEM615_LOGTYPE_RANGECMPB "RANGECMPB "
#define NOVATEL_OEM615_PORT              "COM "
#define NOVATEL_OEM615_PERIOD_OPTION_0   "ONCE"
#define NOVATEL_OEM615_PERIOD_OPTION_1   "ONTIME 0.05"
#define NOVATEL_OEM615_PERIOD_OPTION_2   "ONTIME 0.1"
#define NOVATEL_OEM615_PERIOD_OPTION_3   "ONTIME 0.2"
#define NOVATEL_OEM615_PERIOD_OPTION_4   "ONTIME 0.25"
#define NOVATEL_OEM615_PERIOD_OPTION_5   "ONTIME 0.5"

/*
** NOVATEL_OEM615 device data telemetry definition
*/
typedef struct
{
    uint16_t Weeks;
    uint32_t SecondsIntoWeek;
    double   Fractions;
    double   ECEFX;
    double   ECEFY;
    double   ECEFZ;
    double   VelX;
    double   VelY;
    double   VelZ;
    float    lat;
    float    lon;
    float    alt;

} __attribute__((packed)) NOVATEL_OEM615_Device_Data_tlm_t;
#define NOVATEL_OEM615_DEVICE_DATA_LNGTH sizeof(NOVATEL_OEM615_Device_Data_tlm_t)
#define NOVATEL_OEM615_DEVICE_DATA_SIZE  NOVATEL_OEM615_DEVICE_DATA_LNGTH + NOVATEL_OEM615_DEVICE_HDR_TRL_LEN

// Define the struct to hold GPGGA data
typedef struct
{
    char   utc_time[11];        // hhmmss.ss
    double latitude;            // Decimal degrees
    char   lat_direction;       // 'N' or 'S'
    double longitude;           // Decimal degrees
    char   lon_direction;       // 'E' or 'W'
    int    fix_quality;         // 0 = Invalid, 1 = GPS fix, etc.
    int    num_satellites;      // Number of satellites
    double hdop;                // Horizontal Dilution of Precision
    double altitude;            // Altitude in meters
    char   altitude_units;      // 'M' for meters
    double geoid_separation;    // Height of geoid
    char   geoid_units;         // 'M' for meters
    char   dgps_age[10];        // Time since last DGPS update
    char   dgps_station_id[10]; // DGPS station ID
} __attribute__((packed)) GPGGA_Data_t;
#define GPGGA_DATA_LENGTH sizeof(GPGGA_Data_t);

/*
** Prototypes
*/
int32_t NOVATEL_OEM615_CommandDevice(uart_info_t *uart_device, uint8_t cmd_code, int8_t log_type, int8_t period_option);
int32_t NOVATEL_OEM615_RequestData(uart_info_t *uart_device, NOVATEL_OEM615_Device_Data_tlm_t *data);
int32_t NOVATEL_OEM615_ChildProcessReadData(uart_info_t *uart_device, NOVATEL_OEM615_Device_Data_tlm_t *data);
void    NOVATEL_OEM615_ParseBestXYZA(NOVATEL_OEM615_Device_Data_tlm_t *device_data_struct);
void    NOVATEL_OEM615_ParseBestGPGGA(NOVATEL_OEM615_Device_Data_tlm_t *device_data_struct);

#endif /* _NOVATEL_OEM615_DEVICE_H_ */