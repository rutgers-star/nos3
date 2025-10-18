/* Copyright (C) 2009 - 2017 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S.
Government.

This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including,
but not limited to, any warranty that the software will conform to, specifications any implied warranties of
merchantability, fitness for a particular purpose, and freedom from infringement, and any warranty that the
documentation will conform to the program, or any warranty that the software will be error free.

In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or
consequential damages, arising out of, resulting from, or in any way connected with the software or its documentation.
Whether or not based upon warranty, contract, tort or otherwise, and whether or not loss was sustained from, or arose
out of the results of, or use of, the software, documentation or services provided hereunder

ITC Team
NASA IV&V
ivv-itc@lists.nasa.gov
*/

/*******************************************************************************
** File: cam_device.c
**
** Purpose:
**   This file contains a combination of cam_child and cam_lib.
**
*******************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "cam_device.h"

/*************************************************************************
** Global Data
*************************************************************************/
i2c_bus_info_t CAM_I2C;
spi_info_t     CAM_SPI;

/*******************************************************************************
** Private Function Prototypes
*******************************************************************************/
#ifdef OV5642
static int32_t CAM_setSize_OV5642(void);
#endif

int32_t CAM_init_i2c(void)
{
    uint8_t data[3];
    int32_t result = OS_ERROR;
    uint8_t temp   = 0;
    uint8_t vid, pid;

    CAM_I2C.handle = CAM_I2C_BUS;
    CAM_I2C.isOpen = PORT_CLOSED;
    CAM_I2C.speed  = CAM_SPEED;
    CAM_I2C.addr   = CAM_ADDR;

    i2c_master_init(&CAM_I2C);

    while ((temp < 10) && (result != OS_SUCCESS))
    {
// Change register set to camera
#ifdef OV2640
        data[0] = 0xFF;
        data[1] = 0x01;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, NULL, 0, CAM_TIMEOUT);
#endif
#if (defined(OV5640) || defined(OV5642))
        data[0] = 0x00;
        data[1] = 0xFF;
        data[2] = 0x01;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 3, NULL, 0, CAM_TIMEOUT);
#endif

// Test register 1
#ifdef OV2640
        data[0] = CHIPID_HIGH; // 0x0A
        data[1] = 0x00;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, &vid, 1, CAM_TIMEOUT);
#endif
#if (defined(OV5640) || defined(OV5642))
        data[0] = (CHIPID_HIGH & 0xFF00) >> 8;
        data[1] = (CHIPID_HIGH & 0x00FF);
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, &vid, 1, CAM_TIMEOUT);
#endif

// Test register 2
#ifdef OV2640
        data[0] = CHIPID_LOW; // 0x0B
        data[1] = 0x00;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, &pid, 1, CAM_TIMEOUT);
#endif
#if (defined(OV5640) || defined(OV5642))
        data[0] = (CHIPID_LOW & 0xFF00) >> 8;
        data[1] = (CHIPID_LOW & 0x00FF);
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, &pid, 1, CAM_TIMEOUT);
#endif

#ifdef STF1_DEBUG
        OS_printf("\n vid = 0x%02x; pid = 0x%02x \n", vid, pid);
#endif
        if ((vid == CAM_VID) && (pid == CAM_PID))
        {
            result = OS_SUCCESS;
        }
        temp++;
    }

    return result;
}

int32_t CAM_init_spi(void)
{
    int32_t result          = OS_SUCCESS;
    int32_t state           = OS_SUCCESS;
    uint8_t spir[2]         = {0x00, 0x00};
    uint8_t temp[2]         = {0x00, 0x00};
    uint8_t writereg[2]     = {0x80, 0x55};
    uint8_t readreg[2]      = {0x00, 0x00};
    uint8_t arduchipmode[2] = {0x82, 0x00};

    // Configure SPI
    CAM_SPI.handle   = 0;
    CAM_SPI.baudrate = CAM_SPEED;
    CAM_SPI.cs       = 0;
    CAM_SPI.spi_mode = 0;
    // Setup spi
    result = spi_init_dev(&CAM_SPI);

    // Select chip
    result = spi_select_chip(&CAM_SPI);

    if (result == OS_SUCCESS)
    {
        result = OS_ERROR;
        while ((temp[1] < 5) && (result != OS_SUCCESS))
        {
            // Write value 0x55 into register 0x00
            spi_write(&CAM_SPI, writereg, 2);
            spi_read(&CAM_SPI, spir, 2);
            // Read value at register 0x00
            spi_write(&CAM_SPI, readreg, 2);
            spi_read(&CAM_SPI, spir, 2);
#ifdef STF1_DEBUG
            OS_printf("spir 0x%02x%02x \n", spir[0], spir[1]);
#endif
            // Check if value was successfully written and returned
            if ((spir[1] & 0xFF) != 0x55)
            {
                temp[1]++;
                if (temp[1] == 5)
                {
                    state = OS_ERROR;
                }
            }
            else
            {
                result = OS_SUCCESS;

                // Write and verify lots of values over SPI
                /*
                temp = 0;
                while (temp < 255)
                {
                    // Write data
                    gs_spi_write(&chip, temp | 0x8000);
                    gs_spi_read(&chip, &spir);
                    // Read data
                    gs_spi_write(&chip, temp);
                    gs_spi_read(&chip, &spir);
                    // Verify data
                    if ( (spir & 0x00FF) != temp)
                    {
                        OS_printf("SPI Error - failed on verification of 0x%04x \n", temp);
                    }
                    temp++;
                    OS_TaskDelay(1);
                }
                */
                OS_TaskDelay(100);
                // Change mode - MCU
                spi_write(&CAM_SPI, arduchipmode, 2); // ARDUCHIP_MODE
                OS_TaskDelay(100);
            }
        }

        // Unselect chip
        result = spi_unselect_chip(&CAM_SPI);
        if (result != OS_SUCCESS)
        {
            state = OS_ERROR;
        }
    }

    return state;
}

int32_t CAM_config(void)
{
    uint8_t data[3];
    int32_t result = OS_ERROR;

    // Select chip
    result = spi_select_chip(&CAM_SPI);

    if (result == OS_SUCCESS)
    { // arducam_init()
#ifdef OV2640
      // Change register set to camera
        data[0] = 0xFF;
        data[1] = 0x01;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, NULL, 0, CAM_TIMEOUT);
        // Common control 7
        data[0] = 0x12;
        data[1] = 0x80;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, NULL, 0, CAM_TIMEOUT);
#endif
#ifdef OV5640
        OS_TaskDelay(100);
        data[0] = 0x31;
        data[1] = 0x03;
        data[2] = 0x11;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 3, NULL, 0, CAM_TIMEOUT);
        data[0] = 0x30;
        data[1] = 0x08;
        data[2] = 0x82;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 3, NULL, 0, CAM_TIMEOUT);
#endif
#ifdef OV5642
        data[0] = 0x30;
        data[1] = 0x08;
        data[2] = 0x80;
        i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 3, NULL, 0, CAM_TIMEOUT);
#endif

        OS_TaskDelay(100);

        // Unselect chip
        result = spi_unselect_chip(&CAM_SPI);
        if (result != OS_SUCCESS)
        {
            result = OS_ERROR;
        }
        else
        {
            result = OS_SUCCESS;
        }
    }

    return result;
}

int32_t CAM_setup(void)
{
    int32_t result = OS_SUCCESS;

#ifdef OV2640
    // Change register set to camera
    uint8_t data[2];
    data[0] = 0xFF;
    data[1] = 0x01;
    i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, NULL, 0, CAM_TIMEOUT);
    // Common control 10
    data[0] = 0x15;
    data[1] = 0x00;
    i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 2, NULL, 0, CAM_TIMEOUT);
#endif

    return result;
}

#ifdef OV5642
static int32_t CAM_setSize_OV5642(void)
{
    int32_t result = OS_SUCCESS;
    uint8_t data[3];

    data[0] = 0x38;
    data[1] = 0x18;
    data[2] = 0xA8;
    i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 3, NULL, 0, CAM_TIMEOUT);
    data[0] = 0x36;
    data[1] = 0x21;
    data[2] = 0x10;
    i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 3, NULL, 0, CAM_TIMEOUT);
    data[0] = 0x38;
    data[1] = 0x01;
    data[2] = 0xC8;
    i2c_master_transaction(&CAM_I2C, CAM_ADDR, &data, 3, NULL, 0, CAM_TIMEOUT);

    return result;
}
#endif

int32_t CAM_capture_prep(void)
{
    int32_t result = OS_ERROR;
    uint8_t data[2];

    // Select chip
    result = spi_select_chip(&CAM_SPI);

    if (result == OS_SUCCESS)
    { // Prepare for capture
#if (defined(OV5640) || defined(OV5642))
        data[0] = 0x83;
        data[1] = 0x02;
        spi_write(&CAM_SPI, data, 2); // VSYNC is active HIGH
        OS_TaskDelay(100);
#endif
        data[0] = 0x84;
        data[1] = 0x01;
        spi_write(&CAM_SPI, data, 2); // Flush the fifo
        OS_TaskDelay(100);
        data[0] = 0x84;
        data[1] = 0x01;
        spi_write(&CAM_SPI, data, 2); // Clear capture done flag
        OS_TaskDelay(100);
        data[0] = 0x84;
        data[1] = 0x02;
        spi_write(&CAM_SPI, data, 2); // Start capture
        OS_TaskDelay(100);

        // Unselect chip
        result = spi_unselect_chip(&CAM_SPI);
        if (result != OS_SUCCESS)
        {
            result = OS_ERROR;
        }
        else
        {
            result = OS_SUCCESS;
        }
    }

    return result;
}

int32_t CAM_capture(void)
{
    uint8_t  temp[2] = {0x00, 0x00};
    int32_t  result  = OS_SUCCESS;
    int32_t  state   = OS_SUCCESS;
    uint16_t count   = 0x0000;
    uint8_t  data[2];

    // Select chip
    result = spi_select_chip(&CAM_SPI);

    if (result == OS_SUCCESS)
    { // Wait for capture done
        data[0] = 0x41;
        data[1] = 0x00;
        spi_write(&CAM_SPI, data, 2);
        spi_read(&CAM_SPI, temp, 2);

        while (!(((temp[1] & 0xFF)) & CAP_DONE_MASK))
        {
            data[0] = 0x41;
            data[1] = 0x00;
            spi_write(&CAM_SPI, data, 2);
            spi_read(&CAM_SPI, temp, 2);
            count++;
            OS_TaskDelay(10); // Let other processes run
            // OS_printf("CAM_capture: temp = 0x%04x \n", temp);
            if (count >= 0x0400) // Magic number
            {
                state = OS_ERROR;
                break;
            }
        }

        // Unselect chip
        result = spi_unselect_chip(&CAM_SPI);
        if (result != OS_SUCCESS)
        {
            state = OS_ERROR;
        }
    }

    return state;
}

int32_t CAM_read_fifo_length(uint32_t *length)
{
    int32_t result  = OS_SUCCESS;
    int32_t state   = OS_SUCCESS;
    uint8_t temp[2] = {0x55, 0x55};
    uint8_t data[2];

    // Select chip
    result = spi_select_chip(&CAM_SPI);

    if (result == OS_SUCCESS)
    { // Read FIFO Length
        data[0] = 0x44;
        data[1] = 0x00;
        spi_write(&CAM_SPI, data, 2);
        spi_read(&CAM_SPI, temp, 2);
        // OS_printf("CAM_read_fifo_length: temp = 0x%04x \n", temp);
        *length = (temp[1] & 0x00FF);
        data[0] = 0x43;
        data[1] = 0x00;
        spi_write(&CAM_SPI, data, 2);
        spi_read(&CAM_SPI, temp, 2);
        // OS_printf("CAM_read_fifo_length: temp = 0x%04x \n", temp);
        *length = (*length << 16) | ((temp[1] & 0x00FF) << 8);
        data[0] = 0x42;
        data[1] = 0x00;
        spi_write(&CAM_SPI, data, 2);
        spi_read(&CAM_SPI, temp, 2);
        // OS_printf("CAM_read_fifo_length: temp = 0x%04x \n", temp);
        data[0] = 0x42;
        data[1] = 0x00;
        spi_write(&CAM_SPI, data, 2);
        spi_read(&CAM_SPI, temp, 2);
        // OS_printf("CAM_read_fifo_length: temp = 0x%04x \n", temp);
        *length = (*length | (temp[1] & 0x00FF)) & 0x007FFFFF;
#ifdef STF1_DEBUG
        OS_printf("\n CAM FIFO Length = %d  = 0x%08x\n", (int)*length, (int)*length);
#endif

        if ((*length > MAX_FIFO_SIZE) || (*length == 0))
        {
            state = OS_ERROR;

            state = OS_SUCCESS; // DEBUG
        }

        // Unselect chip
        result = spi_unselect_chip(&CAM_SPI);
        if (result != OS_SUCCESS)
        {
            state = OS_ERROR;
        }
    }

    return state;
}

int32_t CAM_read_prep(char *buf, uint16_t *i)
{
    // Local variables
    uint8_t  temp[2] = {0x00, 0x00};
    int32_t  result  = OS_SUCCESS;
    int32_t  state   = OS_SUCCESS;
    uint16_t count   = 0x0000;
    uint8_t  data[2] = {0x00, 0x00};

#ifdef FILE_OUTPUT
    remove("pic.jpg");
#endif

    // Select chip
    result  = spi_select_chip(&CAM_SPI);
    data[0] = 0xBD;
    data[1] = 0x00;
    spi_write(&CAM_SPI, data, 2);
    spi_read(&CAM_SPI, temp, 2);

    if (result == OS_SUCCESS)
    { // Read until JPEG header
        while (temp[1] != 0xFF)
        {
            data[0] = 0x3D;
            data[1] = 0x00;
            spi_write(&CAM_SPI, data, 2);
            spi_read(&CAM_SPI, temp, 2);
            // OS_printf("CAM_read_prep: temp = 0x%04x \n", temp);
            temp[1] = (temp[1] & 0xFF);

            count++;
            if (count > 500) // Magic Number
            {
                state   = OS_ERROR;
                temp[1] = 0xFF;
            }
        }

        // Unselect chip
        result = spi_unselect_chip(&CAM_SPI);
        if (result != OS_SUCCESS)
        {
            state = OS_ERROR;
        }
        else
        {
            // Write first image data to buffer
            buf[(*i)++] = temp[1];
            result      = OS_SUCCESS;
        }
    }

    return state;
}

int32_t CAM_read(char *buf, uint16_t *i, uint8_t *status)
{
    // Local variables
    uint8_t temp[2]      = {0x00, 0x00};
    uint8_t temp_last[2] = {0x00, 0xFF};
    int32_t result       = OS_SUCCESS;
    uint8_t spiw[2]      = {0x3D, 0x00}; // FIFO read

#ifdef FILE_OUTPUT
    FILE *fp1 = fopen("./pic.jpg", "a");
    if (!fp1)
    {
        OS_printf("\t ERROR: Could not create file\n");
    }
#endif

    // Select chip
    result = spi_select_chip(&CAM_SPI);

    if (result == OS_SUCCESS)
    { // Read JPEG data from FIFO
        while ((((temp[1] != 0xD9) || (temp[0] != 0x00)) || ((temp_last[1] != 0xFF) || (temp_last[0] != 0x00))) &&
               (*i < CAM_DATA_SIZE))
        {
            temp_last[0] = temp[0];
            temp_last[1] = temp[1];
            spiw[0]      = 0x3D;
            spiw[1]      = 0x00;
            spi_write(&CAM_SPI, spiw, 2);
            spi_read(&CAM_SPI, temp, 2);

            // Write image data to buffer
            temp[0]     = (temp[0] & 0x00);
            temp[1]     = (temp[1] & 0xFF);
            buf[(*i)++] = temp[1];

            if (temp_last[0] == 0x00)
            {
                switch (temp_last[1])
                {
                    case 0xFF:
                        if (temp[0] == 0x00)
                        {
                            switch (temp[1])
                            {
                                case 0xD8:
                                    (*status)++;
#ifdef STF1_DEBUG
                                    OS_printf("\n Start of image...\n");
#endif
                                    break;
                                case 0xDA:
                                    (*status)++;
#ifdef STF1_DEBUG
                                    OS_printf("\n Start of scan...\n");
#endif
                                    break;
                                case 0xDB:
                                    (*status)++;
#ifdef STF1_DEBUG
                                    OS_printf("\n Define quantization table(s)...\n");
#endif
                                    break;
                                case 0xC4:
                                    (*status)++;
#ifdef STF1_DEBUG
                                    OS_printf("\n Define huffman table(s)...\n");
#endif
                                    break;
                                case 0xD3:
                                    (*status)++;
#ifdef STF1_DEBUG
                                    OS_printf("\n What is that!?!? \n");
#endif
                                    break;
                                case 0xD9:
                                    (*status)++;
#ifdef STF1_DEBUG
                                    OS_printf("\n End of image...\n");
#endif
                                    (*status) = OS_SUCCESS;
                                    spiw[0]   = 0x84;
                                    spiw[0]   = 0x01; // Clear the capture done flag
                                    spi_write(&CAM_SPI, spiw, 2);
                                    break;
                                default:
                                    break;
                            }
                        } // end if (temp[0] == 0x00)
                        break;
                    default:
                        break;
                }
            } // end if (temp_last[0] == 0x00)
        }

        // Unselect chip
        result = spi_unselect_chip(&CAM_SPI);
        if (result != OS_SUCCESS)
        {
            result = OS_ERROR;
        }
        else
        {
            result = OS_SUCCESS;
        }
    }

// Write the buffer to a file
#ifdef FILE_OUTPUT
    fwrite(buf, *i, 1, fp1);
    fclose(fp1);
#endif

    return result;
}

int take_picture(uint8_t size)
{
    uint8_t  status = 1;
    uint32_t length = 0;
    uint8_t  data[CAM_DATA_SIZE];
    uint16_t x           = 0;
    int32_t  result      = OS_ERROR;
    int32_t  read_result = OS_SUCCESS;

    while (status == 1)
    {
        // Initialize Inter-Integrated Circuit
        result = CAM_init_i2c();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("I2C initialization success\n");

        // Initialize Serial Peripheral Interface
        result = CAM_init_spi();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("SPI initialization success\n");

        // Configure Camera for Upload
        result = CAM_config();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("Configuration success\n");

        // Configure Registers
        result = CAM_jpeg_init();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("JPEG init success\n");

        // Configure Registers
        result = CAM_yuv422();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("YUV422 success\n");

        // Configure Registers
        result = CAM_jpeg();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("JPEG success\n");

        // Configure Camera for Size
        result = CAM_setup();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("Configuration success\n");

        // Upload Size
        result = CAM_setSize(size);
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("Set size success\n");

        // Prepare for Capture
        result = CAM_capture_prep();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("Capture prep success\n");

        // Capture Image
        result = CAM_capture();
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("Capture success\n");

        // Read FIFO Size
        result = CAM_read_fifo_length(&length);
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("Read fifo length success\n");

        // Prepare for FIFO Read
        result = CAM_read_prep((char *)&data, (uint16_t *)&x);
        if (result != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("Read prep success\n");

        //// Read FIFO
        while ((status > 0) && (status <= 8))
        {

            read_result = CAM_read((char *)&data, (uint16_t *)&x, (uint8_t *)&status);

            if (read_result != OS_SUCCESS)
            {
                OS_printf("CAM read error");
            }
            if (read_result != OS_SUCCESS)
                break;
            x = 0;

            OS_TaskDelay(250);
        }

        if (status != OS_SUCCESS)
            return OS_ERROR;
        OS_printf("FIFO success\n");
        break;
    }

    return OS_SUCCESS;
}

/************************/
/*  End of File Comment */
/************************/