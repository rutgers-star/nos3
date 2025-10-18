// ======================================================================
// \title  Generic_fss.cpp
// \author jstar
// \brief  cpp file for Generic_fss component implementation class
// ======================================================================

#include "fss_src/Generic_fss.hpp"
#include "FpConfig.hpp"


namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Generic_fss ::
    Generic_fss(const char* const compName) :
      Generic_fssComponentBase(compName)
  {
    int32_t status = OS_SUCCESS;

    /* Initialize HWLIB */
    nos_init_link();

    /*
    ** Initialize hardware interface data
    */ 
    FssSpi.deviceString = GENERIC_FSS_CFG_STRING;
    FssSpi.handle = GENERIC_FSS_CFG_HANDLE;
    FssSpi.baudrate = GENERIC_FSS_CFG_BAUD;
    FssSpi.spi_mode = GENERIC_FSS_CFG_SPI_MODE;
    FssSpi.bits_per_word = GENERIC_FSS_CFG_BITS_PER_WORD;
    FssSpi.bus = GENERIC_FSS_CFG_BUS;
    FssSpi.cs = GENERIC_FSS_CFG_CS;

    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
    HkTelemetryPkt.DeviceEnabled = GENERIC_FSS_DEVICE_DISABLED;

    /* Open device specific protocols */
    status = spi_init_dev(&FssSpi);
    if (status == OS_SUCCESS)
    {
        printf("SPI device %s configured with baudrate %d \n", FssSpi.deviceString, FssSpi.baudrate);
    }
    else
    {
        printf("SPI device %s failed to initialize! \n", FssSpi.deviceString);
        status = OS_ERROR;
    }

    status = spi_close_device(&FssSpi);

  }

  Generic_fss ::
    ~Generic_fss()
  {
    // Close the device 
    spi_close_device(&FssSpi);

    nos_destroy_link();


  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Generic_fss :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    HkTelemetryPkt.CommandCount++;
    this->log_ACTIVITY_HI_TELEM("NOOP command success!");

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_fss :: ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_FSS_DEVICE_DISABLED)
    {
      HkTelemetryPkt.CommandCount++;

      FssSpi.deviceString = GENERIC_FSS_CFG_STRING;
      FssSpi.handle = GENERIC_FSS_CFG_HANDLE;
      FssSpi.baudrate = GENERIC_FSS_CFG_BAUD;
      FssSpi.spi_mode = GENERIC_FSS_CFG_SPI_MODE;
      FssSpi.bits_per_word = GENERIC_FSS_CFG_BITS_PER_WORD;
      FssSpi.bus = GENERIC_FSS_CFG_BUS;
      FssSpi.cs = GENERIC_FSS_CFG_CS;

      status = spi_init_dev(&FssSpi);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = GENERIC_FSS_DEVICE_ENABLED;
        HkTelemetryPkt.DeviceCount++;

        this->log_ACTIVITY_HI_TELEM("Enable command success!");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;

        this->log_ACTIVITY_HI_TELEM("Enable command failed to init SPI!");
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

  void Generic_fss :: DISABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_FSS_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      status = spi_close_device(&FssSpi);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = GENERIC_FSS_DEVICE_DISABLED;
        HkTelemetryPkt.DeviceCount++;
        this->log_ACTIVITY_HI_TELEM("Disable command success!");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Disable command failed to close SPI!");
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

  void Generic_fss :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    
    if(HkTelemetryPkt.DeviceEnabled == GENERIC_FSS_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      this->tlmWrite_ALPHA(FSSData.Alpha);
      this->tlmWrite_BETA(FSSData.Beta);
      this->tlmWrite_ERRORCODE(FSSData.ErrorCode);
      this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
      this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
      this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
      this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
      this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

      this->log_ACTIVITY_HI_TELEM("Requested Housekeeping!");
    }
    else
    {
      this->log_ACTIVITY_HI_TELEM("HK Failed, Device Disabled!");
    }

    

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_fss :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
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

  void Generic_fss :: REQUEST_DATA_cmdHandler(FwOpcodeType opCode, U32 cmdSeq)
  {
    int32_t status = OS_SUCCESS;


    if(HkTelemetryPkt.DeviceEnabled == GENERIC_FSS_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;
      status = GENERIC_FSS_RequestData(&FssSpi, &FSSData);
      if (status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        this->log_ACTIVITY_HI_TELEM("RequestData command success\n");          
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("RequestData command failed\n");
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("RequestData command failed, Device Disabled\n");

    }

    this->tlmWrite_ALPHA(FSSData.Alpha);
    this->tlmWrite_BETA(FSSData.Beta);
    this->tlmWrite_ERRORCODE(FSSData.ErrorCode);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  inline Generic_fss_ActiveState Generic_fss :: get_active_state(uint8_t DeviceEnabled)
  {
    Generic_fss_ActiveState state;

    if(DeviceEnabled == GENERIC_FSS_DEVICE_ENABLED)
    {
      state.e = Generic_fss_ActiveState::ENABLED;
    }
    else
    {
      state.e = Generic_fss_ActiveState::DISABLED;
    }

    return state;
  }

}
