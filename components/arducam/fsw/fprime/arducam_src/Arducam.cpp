// ======================================================================
// \title  Arducam.cpp
// \author jstar
// \brief  cpp file for Arducam component implementation class
// ======================================================================

#include "arducam_src/Arducam.hpp"
#include "FpConfig.hpp"


namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Arducam ::
    Arducam(const char* const compName) :
      ArducamComponentBase(compName)
  {
    
    nos_init_link();

    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    
  }

  Arducam ::
    ~Arducam()
  {
    // Close the device(s)
    i2c_master_close(&CAM_I2C);
    spi_close_device(&CAM_SPI);

    nos_destroy_link();
    

    OS_printf("Cleanly exiting arducam application...\n\n");
  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Arducam :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    int32_t status = OS_SUCCESS;
    status = CAM_init_i2c();
    if (status != OS_SUCCESS)
    {   
        this->log_ACTIVITY_HI_TELEM("I2C Failure!\n");
        HkTelemetryPkt.CommandErrorCount++;
    }   
    else
    {   
        status = CAM_init_spi();
        if (status != OS_SUCCESS)
        {
            this->log_ACTIVITY_HI_TELEM("SPI Failure!\n");
            HkTelemetryPkt.CommandErrorCount++;
        }
        else
        {
            this->log_ACTIVITY_HI_TELEM("CAM Hardware NOOP (I2C & SPI) Successful!\n");
            HkTelemetryPkt.CommandCount++;
        }
    }   

    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Arducam :: I2C_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    int32_t status = OS_SUCCESS;
    status = CAM_init_i2c();
    if (status == OS_SUCCESS)
    {   
        this->log_ACTIVITY_HI_TELEM("I2C Initialization Success\n");
        HkTelemetryPkt.CommandCount++;
    }   
    else
    {   
        this->log_ACTIVITY_HI_TELEM("I2C Initialization Failed!\n");
        HkTelemetryPkt.CommandErrorCount++;
    }   

    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Arducam :: SPI_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    int32_t status = OS_SUCCESS;
    status = CAM_init_spi();
    if (status == OS_SUCCESS)
    {   
        this->log_ACTIVITY_HI_TELEM("SPI Initialisation Success\n");
        HkTelemetryPkt.CommandCount++;
    }   
    else
    {   
        this->log_ACTIVITY_HI_TELEM("SPI Initialisation Failed!\n");
        HkTelemetryPkt.CommandErrorCount++;
    }   

    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Arducam :: IMAGE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const Arducam_ImageSize image_size) {
    int32_t status = OS_SUCCESS;

    switch (image_size.e)
    {
      case 0:
        status = take_picture(size_320x240);
        break;
      case 1:
        status = take_picture(size_1600x1200);
        break;
      case 2:
        status = take_picture(size_2592x1944);
        break;

      default:
        status = -1;
        break;
    }

    if (status == OS_SUCCESS)
    {   
        this->log_ACTIVITY_HI_TELEM("Arducam image sent\n");
        HkTelemetryPkt.CommandCount++;
    }   
    else
    {   
        this->log_ACTIVITY_HI_TELEM("Arducam image send failed!\n");
        HkTelemetryPkt.CommandErrorCount++;
    }   

    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Arducam :: RESET_COUNTERS_cmdHandler(FwOpcodeType opcode, U32 cmdSeq){
    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;

    this->log_ACTIVITY_HI_TELEM("Reset Command Counters\n");

    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->cmdResponse_out(opcode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Arducam :: REPORT_HOUSEKEEPING_cmdHandler(FwOpcodeType opcode, U32 cmdSeq){
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);

    this->log_ACTIVITY_HI_TELEM("Updated Housekeeping Information\n");

    this->cmdResponse_out(opcode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Arducam :: HARDWARE_CHECKOUT_cmdHandler(FwOpcodeType opcode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;
    status = CAM_init_i2c();
    if (status != OS_SUCCESS)
    {   
        this->log_ACTIVITY_HI_TELEM("I2C Failure!\n");
        HkTelemetryPkt.CommandErrorCount++;
    }   
    else
    {   
        status = CAM_init_spi();
        if (status != OS_SUCCESS)
        {
            this->log_ACTIVITY_HI_TELEM("SPI Failure!\n");
            HkTelemetryPkt.CommandErrorCount++;
        }
        else
        {
            this->log_ACTIVITY_HI_TELEM("CAM Hardware Checkout (I2C & SPI) Successful!\n");
            HkTelemetryPkt.CommandCount++;
        }
    }
    
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->cmdResponse_out(opcode, cmdSeq, Fw::CmdResponse::OK);
  }

}
