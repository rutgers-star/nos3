// ======================================================================
// \title  Generic_torquer.cpp
// \author jstar
// \brief  cpp file for Generic_torquer component implementation class
// ======================================================================

#include <string>
#include "torquer_src/Generic_torquer.hpp"
#include <Fw/Logger/Logger.hpp>
#include "FpConfig.hpp"


namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Generic_torquer ::
    Generic_torquer(const char* const compName) :
      Generic_torquerComponentBase(compName)

  {
    int32_t status = OS_SUCCESS;
    /* Initialize HWLIB */
    nos_init_link();
    
    /* Open device specific protocols */
    
    for(int i = 0; i < 3; i++){
      HkTelemetryPkt.trqHk[i].Direction = 0;
      HkTelemetryPkt.trqHk[i].PercentOn = 0;
      HkTelemetryPkt.trqDevice[i].trq_num = i;
      HkTelemetryPkt.trqDevice[i].timer_period_ns = GENERIC_TORQUER_CFG_PERIOD;
      HkTelemetryPkt.trqDevice[i].timerfd = 0;
      HkTelemetryPkt.trqDevice[i].direction_pin_fd = 0;
      HkTelemetryPkt.trqDevice[i].timer_high_ns = 0;
      HkTelemetryPkt.trqDevice[i].positive_direction = false;
      HkTelemetryPkt.trqDevice[i].enabled = false;

      status = trq_init(&HkTelemetryPkt.trqDevice[i]);
      if (status == OS_SUCCESS)
      {
          printf("Torquer %d initialized successfully \n", i);
      }
      else
      {
          printf("Torquer %d device failed to initialize with error %d!\n", i, status);
      }

    }

    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
    HkTelemetryPkt.DeviceEnabled = GENERIC_TORQUER_DEVICE_DISABLED;

    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    
  }

  Generic_torquer ::
    ~Generic_torquer()
  {
    for(uint8_t i = 0; i < 3; i++)
    {
      trq_close(&HkTelemetryPkt.trqDevice[i]);
    }
    nos_destroy_link();
  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Generic_torquer :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    HkTelemetryPkt.CommandCount++;

    this->log_ACTIVITY_HI_TELEM("NOOP command success!");
    OS_printf("NOOP Command Successful!\n");
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_torquer :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    int32_t status = OS_SUCCESS;

    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;

    this->log_ACTIVITY_HI_TELEM("Reset Counters command successful!");
    OS_printf("Reset Counters command successful!\n");
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_torquer :: ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_TORQUER_DEVICE_DISABLED)
    {
      HkTelemetryPkt.CommandCount++;

      for(uint8_t i = 0; i < 3; i++)
      {
        status += trq_command(&HkTelemetryPkt.trqDevice[i], 0, 0);
        HkTelemetryPkt.trqHk[i].Direction = 0;
        HkTelemetryPkt.trqHk[i].PercentOn = 0;
      }

      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        HkTelemetryPkt.DeviceEnabled = GENERIC_TORQUER_DEVICE_ENABLED;
        this->log_ACTIVITY_HI_TELEM("Enable Torquer Command Successful!");
        OS_printf("Enable Torquer Command Successful!\n");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Enable Failed: Commanding Failed");
        OS_printf("Enable Failed Commanding Torquer\n");
      }

    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Enable Failed: Already Enabled!");
      OS_printf("Enable Failed: Already Enabled!\n");
    }

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->tlmWrite_Percent_0(HkTelemetryPkt.trqHk[0].PercentOn);
    this->tlmWrite_Percent_1(HkTelemetryPkt.trqHk[1].PercentOn);
    this->tlmWrite_Percent_2(HkTelemetryPkt.trqHk[2].PercentOn);

    this->tlmWrite_Direction_0(HkTelemetryPkt.trqHk[0].Direction);
    this->tlmWrite_Direction_1(HkTelemetryPkt.trqHk[1].Direction);
    this->tlmWrite_Direction_2(HkTelemetryPkt.trqHk[2].Direction);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_torquer :: DISABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_TORQUER_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      for(uint8_t i = 0; i < 3; i++)
      {
        trq_command(&HkTelemetryPkt.trqDevice[i], 0, 0);
        HkTelemetryPkt.trqHk[i].Direction = 0;
        HkTelemetryPkt.trqHk[i].PercentOn = 0;
      }

      HkTelemetryPkt.DeviceCount++;
      HkTelemetryPkt.DeviceEnabled = GENERIC_TORQUER_DEVICE_DISABLED;

      this->log_ACTIVITY_HI_TELEM("Disabled Successfully!");
        OS_printf("Disabled Successfully!\n");
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Disable Failed: Already Disabled!");
      OS_printf("Disable Failed: Already Disabled!\n");
    }

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->tlmWrite_Percent_0(HkTelemetryPkt.trqHk[0].PercentOn);
    this->tlmWrite_Percent_1(HkTelemetryPkt.trqHk[1].PercentOn);
    this->tlmWrite_Percent_2(HkTelemetryPkt.trqHk[2].PercentOn);

    this->tlmWrite_Direction_0(HkTelemetryPkt.trqHk[0].Direction);
    this->tlmWrite_Direction_1(HkTelemetryPkt.trqHk[1].Direction);
    this->tlmWrite_Direction_2(HkTelemetryPkt.trqHk[2].Direction);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_torquer :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->tlmWrite_Percent_0(HkTelemetryPkt.trqHk[0].PercentOn);
    this->tlmWrite_Percent_1(HkTelemetryPkt.trqHk[1].PercentOn);
    this->tlmWrite_Percent_2(HkTelemetryPkt.trqHk[2].PercentOn);

    this->tlmWrite_Direction_0(HkTelemetryPkt.trqHk[0].Direction);
    this->tlmWrite_Direction_1(HkTelemetryPkt.trqHk[1].Direction);
    this->tlmWrite_Direction_2(HkTelemetryPkt.trqHk[2].Direction);

    this->log_ACTIVITY_HI_TELEM("Requested Housekeeping!");
    OS_printf("Requested Housekeeping!\n");


    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_torquer :: ALL_CONFIG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U8 Percent_0, U8 Direction_0, U8 Percent_1, U8 Direction_1, U8 Percent_2, U8 Direction_2){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_TORQUER_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      status += GENERIC_TORQUER_Config(&HkTelemetryPkt.trqHk[0], &HkTelemetryPkt.trqDevice[0], Percent_0, Direction_0);
      status += GENERIC_TORQUER_Config(&HkTelemetryPkt.trqHk[1], &HkTelemetryPkt.trqDevice[1], Percent_1, Direction_1);
      status += GENERIC_TORQUER_Config(&HkTelemetryPkt.trqHk[2], &HkTelemetryPkt.trqDevice[2], Percent_2, Direction_2);

      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        this->log_ACTIVITY_HI_TELEM("Successfully Set all Torquers!");
        OS_printf("Successfully Set all Torquers!\n");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Failed to Set All Torquers!");
        OS_printf("Failed to Set All Torquers!\n");
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Failed: Device Disabled");
      OS_printf("Failed: Device Disabled!\n");
    }

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->tlmWrite_Percent_0(HkTelemetryPkt.trqHk[0].PercentOn);
    this->tlmWrite_Percent_1(HkTelemetryPkt.trqHk[1].PercentOn);
    this->tlmWrite_Percent_2(HkTelemetryPkt.trqHk[2].PercentOn);

    this->tlmWrite_Direction_0(HkTelemetryPkt.trqHk[0].Direction);
    this->tlmWrite_Direction_1(HkTelemetryPkt.trqHk[1].Direction);
    this->tlmWrite_Direction_2(HkTelemetryPkt.trqHk[2].Direction);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_torquer :: GENERIC_TORQUER_CONFIG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Generic_torquer_tq_num torquerNum, U8 Percent, U8 Direction) {
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_TORQUER_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      status = GENERIC_TORQUER_Config(&HkTelemetryPkt.trqHk[torquerNum.e], &HkTelemetryPkt.trqDevice[torquerNum.e], Percent, Direction);

      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        char configMsg[40];
        sprintf(configMsg, "Successfully Set Torquer %d!", torquerNum.e);
        this->log_ACTIVITY_HI_TELEM(configMsg);
        OS_printf("Successfully Set Torquer %d!\n", torquerNum.e);
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        char configMsg[40];
        sprintf(configMsg, "Faied to set Torquer %d!", torquerNum.e);
        this->log_ACTIVITY_HI_TELEM(configMsg);
        OS_printf("Failed to Set Torquer %d!\n", torquerNum.e);
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Failed: Device Disabled!");
      OS_printf("Failed: Device Disabled!\n");
    }
    
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->tlmWrite_Percent_0(HkTelemetryPkt.trqHk[0].PercentOn);
    this->tlmWrite_Percent_1(HkTelemetryPkt.trqHk[1].PercentOn);
    this->tlmWrite_Percent_2(HkTelemetryPkt.trqHk[2].PercentOn);

    this->tlmWrite_Direction_0(HkTelemetryPkt.trqHk[0].Direction);
    this->tlmWrite_Direction_1(HkTelemetryPkt.trqHk[1].Direction);
    this->tlmWrite_Direction_2(HkTelemetryPkt.trqHk[2].Direction);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  inline Generic_torquer_ActiveState Generic_torquer :: get_active_state(uint8_t DeviceEnabled){
    Generic_torquer_ActiveState state;

    if(DeviceEnabled == GENERIC_TORQUER_DEVICE_ENABLED)
    {
      state.e = Generic_torquer_ActiveState::ENABLED;
    }
    else
    {
      state.e = Generic_torquer_ActiveState::DISABLED;
    }

    return state;
  }





}
