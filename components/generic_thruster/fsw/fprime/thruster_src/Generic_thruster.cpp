// ======================================================================
// \title  Generic_thruster.cpp
// \author jstar
// \brief  cpp file for Generic_thruster component implementation class
// ======================================================================

#include "thruster_src/Generic_thruster.hpp"
#include "FpConfig.hpp"


namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Generic_thruster ::
    Generic_thruster(const char* const compName) : Generic_thrusterComponentBase(compName)
  {

    int status = OS_SUCCESS;


    /* Initialize HWLIB */
    nos_init_link();

    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
    HkTelemetryPkt.DeviceEnabled = GENERIC_THRUSTER_DEVICE_DISABLED;

    for(int i = 0; i < THRUSTER_NUM; i++) HkTelemetryPkt.Percentage[i] = 0;

    /* Open device specific protocols */
    ThrusterUart.deviceString = GENERIC_THRUSTER_CFG_STRING;
    ThrusterUart.handle = GENERIC_THRUSTER_CFG_HANDLE;
    ThrusterUart.isOpen = PORT_CLOSED;
    ThrusterUart.baud = GENERIC_THRUSTER_CFG_BAUDRATE_HZ;
    status = uart_init_port(&ThrusterUart);
    if (status == OS_SUCCESS)
    {
        printf("UART device %s configured with baudrate %d \n", ThrusterUart.deviceString, ThrusterUart.baud);
    }
    else
    {
        printf("UART device %s failed to initialize! \n", ThrusterUart.deviceString);
    }

    status = uart_close_port(&ThrusterUart);
  }

  Generic_thruster ::
    ~Generic_thruster()
  {
    uart_close_port(&ThrusterUart);

    nos_destroy_link();

  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Generic_thruster :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    HkTelemetryPkt.CommandCount++;

    this->log_ACTIVITY_HI_TELEM("NOOP command success!");
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_thruster :: ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_THRUSTER_DEVICE_DISABLED)
    {
      HkTelemetryPkt.CommandCount++;

      ThrusterUart.deviceString = GENERIC_THRUSTER_CFG_STRING;
      ThrusterUart.handle = GENERIC_THRUSTER_CFG_HANDLE;
      ThrusterUart.isOpen = PORT_CLOSED;
      ThrusterUart.baud = GENERIC_THRUSTER_CFG_BAUDRATE_HZ;
      status = uart_init_port(&ThrusterUart);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        HkTelemetryPkt.DeviceEnabled = GENERIC_THRUSTER_DEVICE_ENABLED;
        this->log_ACTIVITY_HI_TELEM("Enable command success!");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Enable command failed to init UART!");
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Enable failed, already Enabled!");
    }

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_thruster :: DISABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_THRUSTER_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      status = uart_close_port(&ThrusterUart);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        HkTelemetryPkt.DeviceEnabled = GENERIC_THRUSTER_DEVICE_DISABLED;
        this->log_ACTIVITY_HI_TELEM("Disable command success!");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Disable command failed to close UART!");
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Disable failed, already Disabled!");
    }

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_thruster :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
    this->log_ACTIVITY_HI_TELEM("Reset Counters command successful!");
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_thruster :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    if(HkTelemetryPkt.DeviceEnabled == GENERIC_THRUSTER_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
      this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
      this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
      this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
      this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
      this->tlmWrite_Percentage_0(HkTelemetryPkt.Percentage[0]);
      this->tlmWrite_Percentage_1(HkTelemetryPkt.Percentage[1]);
      this->tlmWrite_Percentage_2(HkTelemetryPkt.Percentage[2]);
      this->tlmWrite_Percentage_3(HkTelemetryPkt.Percentage[3]);

      this->log_ACTIVITY_HI_TELEM("Requested Housekeeping!");
    }
    else
    {
      this->log_ACTIVITY_HI_TELEM("HK Failed, Device Disabled!");
    }   

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_thruster :: SET_PERCENTAGE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const U8 percent, const Generic_thruster_thrusterNums thruster_number){
    
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_THRUSTER_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      status = GENERIC_THRUSTER_SetPercentage(&ThrusterUart, thruster_number.e, percent, GENERIC_THRUSTER_DEVICE_CMD_SIZE);
      if (status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        HkTelemetryPkt.Percentage[thruster_number.e] = percent;

        char configMsg[40];
        sprintf(configMsg, "Thruster %d set to %d%% successfully!", thruster_number.e, percent);
        this->log_ACTIVITY_HI_TELEM(configMsg);
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        char configMsg[40];
        sprintf(configMsg, "Failed to set Thruster %d to %d%%!", thruster_number.e, percent);
        this->log_ACTIVITY_HI_TELEM(configMsg);
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;

      this->log_ACTIVITY_HI_TELEM("Command failed, device Disabled!");
    }

    this->tlmWrite_Percentage_0(HkTelemetryPkt.Percentage[0]);
    this->tlmWrite_Percentage_1(HkTelemetryPkt.Percentage[1]);
    this->tlmWrite_Percentage_2(HkTelemetryPkt.Percentage[2]);
    this->tlmWrite_Percentage_3(HkTelemetryPkt.Percentage[3]);

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
        
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  inline Generic_thruster_ActiveState Generic_thruster :: get_active_state(uint8_t DeviceEnabled)
  {
    Generic_thruster_ActiveState state;

    if(DeviceEnabled == GENERIC_THRUSTER_DEVICE_ENABLED)
    {
      state.e = Generic_thruster_ActiveState::ENABLED;
    }
    else
    {
      state.e = Generic_thruster_ActiveState::DISABLED;
    }

    return state;
  }

}
