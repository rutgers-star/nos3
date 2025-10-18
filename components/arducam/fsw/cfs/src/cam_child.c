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

#include "cam_child.h"

/*
**  Name:  CAM_publish
**
**  Purpose:
** 		   Break apart functionality, publish received data.
*/
int32_t CAM_publish(void)
{
    OS_MutSemTake(CAM_AppData.data_mutex);
    CAM_AppData.Exp_Pkt.msg_count++;
    CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&CAM_AppData.Exp_Pkt);
    CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&CAM_AppData.Exp_Pkt, true);
    OS_MutSemGive(CAM_AppData.data_mutex);
    return OS_SUCCESS;
} /* End of CAM_publish() */

/*
**  Name:  CAM_state
**
**  Purpose:
** 		   	Checks the state of the experiment
**			Holds on pause and quits on stop
*/
int32_t CAM_state(void)
{
    int32_t  result = OS_ERROR;
    uint32_t state;

    OS_MutSemTake(CAM_AppData.data_mutex);
    state = CAM_AppData.State;
    OS_MutSemGive(CAM_AppData.data_mutex);

    switch (state)
    {
        case CAM_LOW_VOLTAGE:
            OS_printf("CAM child task low voltage received");
            break;

        case CAM_TIME:
            OS_printf("CAM child task timeout received");
            break;

        case CAM_STOP:
            // Do nothing
            break;

        case CAM_PAUSE:
            while (state == CAM_PAUSE)
            {
                OS_MutSemTake(CAM_AppData.data_mutex);
                state = CAM_AppData.State;
                OS_MutSemGive(CAM_AppData.data_mutex);
                OS_TaskDelay(1000);
            }
            if (state == CAM_STOP)
            {
                result = OS_ERROR;
            }
            result = OS_SUCCESS;
            break;

        default: // CAM_RUN
            result = OS_SUCCESS;
    }
    return result;
}

/*
**  Name:  CAM_fifo
**
**  Purpose:
** 		   Read the camera FIFO until commanded to stop, complete, or error occurs.
*/
int32_t CAM_fifo(uint16 *x, uint8 *status)
{
    int32_t result = OS_SUCCESS;

    while ((*status > 0) && (*status <= 8) &&
           (CAM_AppData.Exp_Pkt.msg_count < ((CAM_AppData.Exp_Pkt.length / CAM_DATA_SIZE) + 1)))
    // Status is used to track key points such as start and end of the image
    // Limiting this number ensures that cycling through the FIFO repeatedly is avoided
    {
        // Read a packet
        OS_MutSemTake(CAM_AppData.data_mutex);
        result = CAM_read((char *)&CAM_AppData.Exp_Pkt.data, x, status);
        OS_MutSemGive(CAM_AppData.data_mutex);
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM read error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;
        (*x) = 0;

        // Publish the packet
        result = CAM_publish();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM publish error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Delay between messages to allow for processing
        OS_TaskDelay(250);

#ifdef STF1_DEBUG
        OS_MutSemTake(CAM_AppData.data_mutex);
        OS_printf("\n status   = %d \n", *status);
        OS_printf("\n msg_count = %d \n", CAM_AppData.Exp_Pkt.msg_count);
        OS_MutSemGive(CAM_AppData.data_mutex);
#endif
    }
    return result;
}

/*
**  Name:  CAM_exp
**
**  Purpose:
** 		   The experiment runs until the parent dies or is commanded to stop by the parent
** 		   but has the ability to be paused and resumed depending on the current state.
*/
int32_t CAM_exp(void)
{
    int32_t result = OS_ERROR;
    uint8   status = 1;
    uint16  x      = 0;

    while (status == 1)
    { // Check state
        if (CAM_state() != OS_SUCCESS)
            break;

        // Initialize Serial Peripheral Interface
        result = CAM_init_spi();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM init spi error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Initialize Inter-Integrated Circuit
        result = CAM_init_i2c();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM init i2c error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Configure Camera for Upload
        result = CAM_config();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM configure camera for upload error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Configure Registers
        result = CAM_jpeg_init();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM jpeg init error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Configure Registers
        result = CAM_yuv422();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM yuv422 error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Configure Registers
        result = CAM_jpeg();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM jpeg error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Configure Camera for Size
        result = CAM_setup();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM setup error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Upload Size
        result = CAM_setSize(CAM_AppData.Size);
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM upload size error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Prepare for Capture
        result = CAM_capture_prep();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM capture prep error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Capture Image
        result = CAM_capture();
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM capture error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Read FIFO Size
        result = CAM_read_fifo_length(&CAM_AppData.Exp_Pkt.length);
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM read fifo length error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Prepare for FIFO Read
        OS_MutSemTake(CAM_AppData.data_mutex);
        CAM_AppData.Exp_Pkt.msg_count = 0x0000;
        result                        = CAM_read_prep((char *)&CAM_AppData.Exp_Pkt.data, (uint16 *)&x);
        OS_MutSemGive(CAM_AppData.data_mutex);
        if (result != OS_SUCCESS)
        {
            OS_printf("CAM read prep error");
            OS_MutSemTake(CAM_AppData.data_mutex);
            CAM_AppData.State = CAM_STOP;
            OS_MutSemGive(CAM_AppData.data_mutex);
        }
        if (CAM_state() != OS_SUCCESS)
            break;

        // Read FIFO
        result = CAM_fifo((uint16 *)&x, (uint8 *)&status);
        break;
    }

    return result;
}

/*
** CAM Child Task Startup Initialization
*/
int32_t CAM_ChildInit(void)
{
    int32_t result;

    /* Create child task (low priority command handler) */
    result = CFE_ES_CreateChildTask(&CAM_AppData.ChildTaskID, CAM_CHILD_TASK_NAME, CAM_ChildTask, 0,
                                    CAM_CHILD_TASK_STACK_SIZE, CAM_CHILD_TASK_PRIORITY, 0);

    if (result != CFE_SUCCESS)
    {
        OS_printf("CAM child task initialization error: create task failed: result = %d", result);
    }

    return result;
} /* End of CAM_ChildInit() */

/*
**  Name:  CAM_ChildTask
**
**  Purpose:
** 		   The child task remains active until provided the binary semaphore by the parent
**         when and experiment is kicked off.
*/
void CAM_ChildTask(void)
{
    int32_t result;
    int32_t state;

    OS_printf("CAM child task initialization complete");

    while (true)
    {
        // Block on Semaphore
        OS_BinSemTake(CAM_AppData.sem_id);

        // Check State
        OS_MutSemTake(CAM_AppData.data_mutex);
        state = CAM_AppData.State;
        OS_MutSemGive(CAM_AppData.data_mutex);
        if (state == CAM_PAUSE)
        {
            while (CAM_state() != OS_SUCCESS)
                ;
        }

        // Initialize Child Process Flags
        OS_MutSemTake(CAM_AppData.data_mutex);
        CAM_AppData.State = CAM_RUN;
        switch (CAM_AppData.Exp)
        {
            case 1:
#ifdef OV2640
                CAM_AppData.Size = size_160x120;
#endif
#ifdef OV5640
                CAM_AppData.Size = size_320x240;
#endif
#ifdef OV5642
                CAM_AppData.Size = size_320x240;
#endif
                break;
            case 2:
#ifdef OV2640
                CAM_AppData.Size = size_800x600;
#endif
#ifdef OV5640
                CAM_AppData.Size = size_1600x1200;
#endif
#ifdef OV5642
                CAM_AppData.Size = size_1600x1200;
#endif
                break;
            case 3:
#ifdef OV2640
                CAM_AppData.Size = size_1600x1200;
#endif
#ifdef OV5640
                CAM_AppData.Size = size_2592x1944;
#endif
#ifdef OV5642
                CAM_AppData.Size = size_2592x1944;
#endif
                break;
            default:
                OS_printf("CAM experiment ID error");
                CAM_AppData.State = CAM_STOP;
                break;
        }
        OS_MutSemGive(CAM_AppData.data_mutex);

        // Run Experiment
        result = CAM_exp();
        // Check Result
        OS_MutSemTake(CAM_AppData.data_mutex);
        if ((result == OS_SUCCESS) && (CAM_AppData.State == CAM_RUN))
        {
            switch (CAM_AppData.Exp)
            {
                case 1:
                    // OS_printf("CAM EXP1 Complete\n");
                    CFE_EVS_SendEvent(CAM_EXP3_EID, CFE_EVS_EventType_INFORMATION, "CAM App: EXP 1 Completed");
                    break;
                case 2:
                    // OS_printf("CAM EXP2 Complete\n");
                    CFE_EVS_SendEvent(CAM_EXP3_EID, CFE_EVS_EventType_INFORMATION, "CAM App: EXP 2 Completed");
                    break;
                case 3:
                    // OS_printf("CAM EXP3 Complete\n");
                    CFE_EVS_SendEvent(CAM_EXP3_EID, CFE_EVS_EventType_INFORMATION, "CAM App: EXP 3 Completed");
                    break;
                default:
                    break;
            }
            // Delay to allow for all CAM Tlm messages to be cleared from pipe
            OS_TaskDelay(10000);
        }
        // Cleanup
        CAM_AppData.State = CAM_STOP;
        OS_MutSemGive(CAM_AppData.data_mutex);
    }

    /* This call allows cFE to clean-up system resources */
    OS_printf("CAM child task exit complete");
    CFE_ES_ExitChildTask();
} /* End of CAM_ChildTask() */
