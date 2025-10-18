// ======================================================================
// \title  SampleSim.cpp
// \author jstar
// \brief  cpp file for SampleSim component implementation class
// ======================================================================

#include "sample_src/SampleSim.hpp"
#include <Fw/Logger/Logger.hpp>
#include "FpConfig.hpp"

namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  SampleSim ::
    SampleSim(const char *const compName) : SampleSimComponentBase(compName)
{
    SampleUart.deviceString = SAMPLE_CFG_STRING;
    SampleUart.handle = SAMPLE_CFG_HANDLE;
    SampleUart.isOpen = PORT_CLOSED;
    SampleUart.baud = SAMPLE_CFG_BAUDRATE_HZ;
    status = uart_init_port(&SampleUart);
    status = uart_close_port(&SampleUart);

    HkTelemetryPkt.DeviceEnabled = SAMPLE_DEVICE_DISABLED;
    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
}
  
  SampleSim ::
    ~SampleSim()
  {
      status = uart_close_port(&SampleUart);
  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void SampleSim :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    status = SAMPLE_CommandDevice(&SampleUart, SAMPLE_DEVICE_NOOP_CMD, 0);
    this->log_ACTIVITY_HI_TELEM("NOOP SENT");
    OS_printf("NOOP SENT\n");

    this->tlmWrite_CommandCount(++HkTelemetryPkt.CommandCount);

    this->tlmWrite_ReportedComponentCount(SampleHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(SampleHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(SampleHK.DeviceStatus);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void SampleSim :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
    if(HkTelemetryPkt.DeviceEnabled == SAMPLE_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;
      status = SAMPLE_RequestHK(&SampleUart, &SampleHK);
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
    
    this->tlmWrite_ReportedComponentCount(SampleHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(SampleHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(SampleHK.DeviceStatus);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void SampleSim :: SAMPLE_SEQ_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
  // seq_toggle = 1;
  
    for(int i=0;i<20;i++){
    // while(1){
      sleep(1);
      // printf("seq toggle is equal to %d \n", seq_toggle);
      // if(seq_toggle==0){
      //   break;
      // }

      if(HkTelemetryPkt.DeviceEnabled == SAMPLE_DEVICE_ENABLED)
      {
        HkTelemetryPkt.CommandCount++;
        status = SAMPLE_RequestHK(&SampleUart, &SampleHK);
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
      
      this->tlmWrite_ReportedComponentCount(SampleHK.DeviceCounter);
      this->tlmWrite_DeviceConfig(SampleHK.DeviceConfig);
      this->tlmWrite_DeviceStatus(SampleHK.DeviceStatus);
      this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
      this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
      this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
      this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
      this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    }

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void SampleSim :: ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    if(HkTelemetryPkt.DeviceEnabled == SAMPLE_DEVICE_DISABLED)
    {

      HkTelemetryPkt.CommandCount++;
      
      SampleUart.deviceString  = SAMPLE_CFG_STRING;
      SampleUart.handle        = SAMPLE_CFG_HANDLE;
      SampleUart.isOpen        = PORT_CLOSED;
      SampleUart.baud          = SAMPLE_CFG_BAUDRATE_HZ;
      SampleUart.access_option = uart_access_flag_RDWR;

      status = uart_init_port(&SampleUart);
      if(status == OS_SUCCESS)
      {

        HkTelemetryPkt.DeviceEnabled = SAMPLE_DEVICE_ENABLED;
        HkTelemetryPkt.DeviceCount++;
        
        this->log_ACTIVITY_HI_TELEM("Successfully Enabled");  
        OS_printf("SampleSim Enable Succeeded\n");  
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Enable failed, failed to init UART port");  
        OS_printf("SampleSim Enable Failed to init UART port\n");  
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Failed, Already Enabled");  
      OS_printf("SampleSim Enable Failed, Already Enabled\n");
    }

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->tlmWrite_ReportedComponentCount(SampleHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(SampleHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(SampleHK.DeviceStatus);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void SampleSim :: DISABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    if(HkTelemetryPkt.DeviceEnabled == SAMPLE_DEVICE_ENABLED)
    {

      HkTelemetryPkt.CommandCount++;

      status = uart_close_port(&SampleUart);
      if (status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = SAMPLE_DEVICE_DISABLED;
        HkTelemetryPkt.DeviceCount++;

        this->log_ACTIVITY_HI_TELEM("Disabled Successfully");  
        OS_printf("SampleSim Disable Succeeded\n");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Disable Failed to close UART port");  
        OS_printf("SampleSim Disable Failed to close UART port\n");
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Failed, Already Disabled");  
      OS_printf("SampleSim Disable Failed, device already disabled\n");
    }

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->tlmWrite_ReportedComponentCount(SampleHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(SampleHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(SampleHK.DeviceStatus);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void SampleSim :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
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

  void SampleSim :: CONFIGURE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const U32 config){

    status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled != SAMPLE_DEVICE_ENABLED)
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

      status = SAMPLE_CommandDevice(&SampleUart, SAMPLE_DEVICE_CFG_CMD, config);
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
    this->tlmWrite_ReportedComponentCount(SampleHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(SampleHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(SampleHK.DeviceStatus);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);

  }

  inline SampleSim_ActiveState SampleSim :: get_active_state(uint8_t DeviceEnabled)
  {
    SampleSim_ActiveState state;

    if(DeviceEnabled == SAMPLE_DEVICE_ENABLED)
    {
      state.e = SampleSim_ActiveState::ENABLED;
    }
    else
    {
      state.e = SampleSim_ActiveState::DISABLED;
    }

    return state;
  }

  //  void SampleSim :: SAMPLE_SEQ_CANCEL_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
  //   seq_toggle = 0;
  //   printf("seq toggle is equal to %d\n", seq_toggle);

  //   this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  // }

}