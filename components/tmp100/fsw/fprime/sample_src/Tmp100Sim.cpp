// ======================================================================
// \title  Tmp100Sim.cpp
// \author jstar
// \brief  cpp file for Tmp100Sim component implementation class
// ======================================================================

#include "tmp100_src/Tmp100Sim.hpp"
#include <Fw/Logger/Logger.hpp>
#include "FpConfig.hpp"

namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Tmp100Sim ::
    Tmp100Sim(const char *const compName) : Tmp100SimComponentBase(compName)
{
    Tmp100Uart.deviceString = TMP100_CFG_STRING;
    Tmp100Uart.handle = TMP100_CFG_HANDLE;
    Tmp100Uart.isOpen = PORT_CLOSED;
    Tmp100Uart.baud = TMP100_CFG_BAUDRATE_HZ;
    status = uart_init_port(&Tmp100Uart);
    status = uart_close_port(&Tmp100Uart);

    HkTelemetryPkt.DeviceEnabled = TMP100_DEVICE_DISABLED;
    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
}
  
  Tmp100Sim ::
    ~Tmp100Sim()
  {
      status = uart_close_port(&Tmp100Uart);
  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Tmp100Sim :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    status = TMP100_CommandDevice(&Tmp100Uart, TMP100_DEVICE_NOOP_CMD, 0);
    this->log_ACTIVITY_HI_TELEM("NOOP SENT");
    OS_printf("NOOP SENT\n");

    this->tlmWrite_CommandCount(++HkTelemetryPkt.CommandCount);

    this->tlmWrite_ReportedComponentCount(Tmp100HK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Tmp100HK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Tmp100HK.DeviceStatus);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Tmp100Sim :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
    if(HkTelemetryPkt.DeviceEnabled == TMP100_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;
      status = TMP100_RequestHK(&Tmp100Uart, &Tmp100HK);
      if (status == OS_SUCCESS)
      {
          HkTelemetryPkt.DeviceCount++;
          this->log_ACTIVITY_HI_TELEM("RequestHK command success\n");
          OS_printf("Request Housekeeping Successful\n");
      }
      else
      {
          HkTelemetryPkt.DeviceErrorCount++;
          this->log_ACTIVITY_HI_TELEM("RequestHK command failed!\n");
          OS_printf("Request Housekeeping Failed\n");
      }

    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("RequestHK failed: Device Disabled");
      OS_printf("Request Housekeeping failed, Device Disabled\n");
    }
    
    this->tlmWrite_ReportedComponentCount(Tmp100HK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Tmp100HK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Tmp100HK.DeviceStatus);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Tmp100Sim :: TMP100_SEQ_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
  // seq_toggle = 1;
  
    for(int i=0;i<20;i++){
    // while(1){
      sleep(1);
      // printf("seq toggle is equal to %d \n", seq_toggle);
      // if(seq_toggle==0){
      //   break;
      // }

      if(HkTelemetryPkt.DeviceEnabled == TMP100_DEVICE_ENABLED)
      {
        HkTelemetryPkt.CommandCount++;
        status = TMP100_RequestHK(&Tmp100Uart, &Tmp100HK);
        if (status == OS_SUCCESS)
        {
            HkTelemetryPkt.DeviceCount++;
            this->log_ACTIVITY_HI_TELEM("RequestHK command success\n");
        }
        else
        {
            HkTelemetryPkt.DeviceErrorCount++;
            this->log_ACTIVITY_HI_TELEM("RequestHK command failed!\n");
        }

      }
      else
      {
        HkTelemetryPkt.CommandErrorCount++;
        this->log_ACTIVITY_HI_TELEM("RequestHK failed: Device Disabled");
      }
      
      this->tlmWrite_ReportedComponentCount(Tmp100HK.DeviceCounter);
      this->tlmWrite_DeviceConfig(Tmp100HK.DeviceConfig);
      this->tlmWrite_DeviceStatus(Tmp100HK.DeviceStatus);
      this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
      this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
      this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
      this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
      this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    }

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Tmp100Sim :: ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    if(HkTelemetryPkt.DeviceEnabled == TMP100_DEVICE_DISABLED)
    {

      HkTelemetryPkt.CommandCount++;
      
      Tmp100Uart.deviceString  = TMP100_CFG_STRING;
      Tmp100Uart.handle        = TMP100_CFG_HANDLE;
      Tmp100Uart.isOpen        = PORT_CLOSED;
      Tmp100Uart.baud          = TMP100_CFG_BAUDRATE_HZ;
      Tmp100Uart.access_option = uart_access_flag_RDWR;

      status = uart_init_port(&Tmp100Uart);
      if(status == OS_SUCCESS)
      {

        HkTelemetryPkt.DeviceEnabled = TMP100_DEVICE_ENABLED;
        HkTelemetryPkt.DeviceCount++;
        
        this->log_ACTIVITY_HI_TELEM("Successfully Enabled");  
        OS_printf("Tmp100Sim Enable Succeeded\n");  
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Enable failed, failed to init UART port");  
        OS_printf("Tmp100Sim Enable Failed to init UART port\n");  
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Failed, Already Enabled");  
      OS_printf("Tmp100Sim Enable Failed, Already Enabled\n");
    }

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->tlmWrite_ReportedComponentCount(Tmp100HK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Tmp100HK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Tmp100HK.DeviceStatus);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Tmp100Sim :: DISABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    if(HkTelemetryPkt.DeviceEnabled == TMP100_DEVICE_ENABLED)
    {

      HkTelemetryPkt.CommandCount++;

      status = uart_close_port(&Tmp100Uart);
      if (status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = TMP100_DEVICE_DISABLED;
        HkTelemetryPkt.DeviceCount++;

        this->log_ACTIVITY_HI_TELEM("Disabled Successfully");  
        OS_printf("Tmp100Sim Disable Succeeded\n");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Disable Failed to close UART port");  
        OS_printf("Tmp100Sim Disable Failed to close UART port\n");
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Failed, Already Disabled");  
      OS_printf("Tmp100Sim Disable Failed, device already disabled\n");
    }

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->tlmWrite_ReportedComponentCount(Tmp100HK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Tmp100HK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Tmp100HK.DeviceStatus);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Tmp100Sim :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);

    this->log_ACTIVITY_HI_TELEM("Counters have been Reset");
    OS_printf("Counters have been Reset\n");

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);

  }

  void Tmp100Sim :: CONFIGURE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const U32 config){

    status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled != TMP100_DEVICE_ENABLED)
    {
      status = OS_ERROR;

      HkTelemetryPkt.CommandErrorCount++;

      this->log_ACTIVITY_HI_TELEM("Configure Failed, Device Disabled");
      OS_printf("Configure Failed, Device Disabled\n");
    }

    if(config == 0xFFFFFFFF) // 4294967295
    {
      status = OS_ERROR;

      HkTelemetryPkt.CommandErrorCount++;

      this->log_ACTIVITY_HI_TELEM("Configure Failed, Invalid Configuration");
      OS_printf("Configure Failed, Invalid Configuration Given\n");
    }

    if(status == OS_SUCCESS)
    {
      HkTelemetryPkt.CommandCount++;

      status = TMP100_CommandDevice(&Tmp100Uart, TMP100_DEVICE_CFG_CMD, config);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        this->log_ACTIVITY_HI_TELEM("Successfully Configured Device");
        OS_printf("Device Successfully Configured\n");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Failed to Configure Device");
        OS_printf("Device Configuration Failed\n");
      }
    }

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_ReportedComponentCount(Tmp100HK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Tmp100HK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Tmp100HK.DeviceStatus);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);

  }

  inline Tmp100Sim_ActiveState Tmp100Sim :: get_active_state(uint8_t DeviceEnabled)
  {
    Tmp100Sim_ActiveState state;

    if(DeviceEnabled == TMP100_DEVICE_ENABLED)
    {
      state.e = Tmp100Sim_ActiveState::ENABLED;
    }
    else
    {
      state.e = Tmp100Sim_ActiveState::DISABLED;
    }

    return state;
  }

  //  void Tmp100Sim :: TMP100_SEQ_CANCEL_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
  //   seq_toggle = 0;
  //   printf("seq toggle is equal to %d\n", seq_toggle);

  //   this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  // }

}