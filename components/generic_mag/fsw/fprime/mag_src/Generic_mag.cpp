// ======================================================================
// \title  Generic_mag.cpp
// \author jstar
// \brief  cpp file for Generic_mag component implementation class
// ======================================================================

#include "mag_src/Generic_mag.hpp"
#include "FpConfig.hpp"


namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Generic_mag ::
    Generic_mag(const char* const compName) :
      Generic_magComponentBase(compName)
  {
    uint32_t status = OS_SUCCESS;

    nos_init_link();

    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
    HkTelemetryPkt.DeviceEnabled = GENERIC_MAG_DEVICE_DISABLED;

    /* Open device specific protocols */
    Generic_magSpi.deviceString = GENERIC_MAG_CFG_STRING;
    Generic_magSpi.handle = GENERIC_MAG_CFG_HANDLE;
    Generic_magSpi.baudrate = GENERIC_MAG_CFG_BAUD;
    Generic_magSpi.spi_mode = GENERIC_MAG_CFG_SPI_MODE;
    Generic_magSpi.bits_per_word = GENERIC_MAG_CFG_BITS_PER_WORD;
    Generic_magSpi.bus = GENERIC_MAG_CFG_BUS;
    Generic_magSpi.cs = GENERIC_MAG_CFG_CS;
    status = spi_init_dev(&Generic_magSpi);
    if (status == OS_SUCCESS)
    {
        printf("SPI device %s configured with baudrate %d \n", Generic_magSpi.deviceString, Generic_magSpi.baudrate);
    }
    else
    {
        printf("SPI device %s failed to initialize! \n", Generic_magSpi.deviceString);
        status = OS_ERROR;
    }

    status = spi_close_device(&Generic_magSpi);


    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
  }

  Generic_mag ::
    ~Generic_mag()
  {
    uint32_t status = OS_SUCCESS;
    
    status = spi_close_device(&Generic_magSpi);

    nos_destroy_link();

  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Generic_mag :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    HkTelemetryPkt.CommandCount++;

    this->log_ACTIVITY_HI_TELEM("NOOP command success!");
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_mag :: ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_MAG_DEVICE_DISABLED)
    {
      HkTelemetryPkt.CommandCount++;

      Generic_magSpi.deviceString = GENERIC_MAG_CFG_STRING;
      Generic_magSpi.handle = GENERIC_MAG_CFG_HANDLE;
      Generic_magSpi.baudrate = GENERIC_MAG_CFG_BAUD;
      Generic_magSpi.spi_mode = GENERIC_MAG_CFG_SPI_MODE;
      Generic_magSpi.bits_per_word = GENERIC_MAG_CFG_BITS_PER_WORD;
      Generic_magSpi.bus = GENERIC_MAG_CFG_BUS;
      Generic_magSpi.cs = GENERIC_MAG_CFG_CS;
      status = spi_init_dev(&Generic_magSpi);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = GENERIC_MAG_DEVICE_ENABLED;
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

  void Generic_mag :: DISABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_MAG_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      status = spi_close_device(&Generic_magSpi);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = GENERIC_MAG_DEVICE_DISABLED;
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

  void Generic_mag :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
    if(HkTelemetryPkt.DeviceEnabled == GENERIC_MAG_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      this->tlmWrite_MagneticIntensityX(Generic_magData.MagneticIntensityX);
      this->tlmWrite_MagneticIntensityY(Generic_magData.MagneticIntensityY);
      this->tlmWrite_MagneticIntensityZ(Generic_magData.MagneticIntensityZ);
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

  void Generic_mag :: REQUEST_DATA_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) 
  {
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_MAG_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;
      status = GENERIC_MAG_RequestData(&Generic_magSpi, &Generic_magData);
      if (status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        this->log_ACTIVITY_HI_TELEM("RequestData command success\n");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("RequestData command failed!\n");
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("RequestData command failed, device disabled!\n");
    }
    
    this->tlmWrite_MagneticIntensityX(Generic_magData.MagneticIntensityX);
    this->tlmWrite_MagneticIntensityY(Generic_magData.MagneticIntensityY);
    this->tlmWrite_MagneticIntensityZ(Generic_magData.MagneticIntensityZ);

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    
    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_mag :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
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

  inline Generic_mag_ActiveState Generic_mag :: get_active_state(uint8_t DeviceEnabled)
  {
    Generic_mag_ActiveState state;

    if(DeviceEnabled == GENERIC_MAG_DEVICE_ENABLED)
    {
      state.e = Generic_mag_ActiveState::ENABLED;
    }
    else
    {
      state.e = Generic_mag_ActiveState::DISABLED;
    }

    return state;
  }

}
