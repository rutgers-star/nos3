// ======================================================================
// \title  Generic_imu.cpp
// \author jstar
// \brief  cpp file for Generic_imu component implementation class
// ======================================================================

#include "imu_src/Generic_imu.hpp"
#include "FpConfig.hpp"


namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Generic_imu ::
    Generic_imu(const char* const compName) :
      Generic_imuComponentBase(compName)
  {
    nos_init_link();

    int32_t status = OS_SUCCESS;
    /* Open device specific protocols */
    Generic_IMUcan.handle = GENERIC_IMU_CFG_HANDLE;
    Generic_IMUcan.isUp = CAN_INTERFACE_DOWN;
    Generic_IMUcan.loopback = false;
    Generic_IMUcan.listenOnly = false;
    Generic_IMUcan.tripleSampling = false;
    Generic_IMUcan.oneShot = false;
    Generic_IMUcan.berrReporting = false;
    Generic_IMUcan.fd = false;
    Generic_IMUcan.presumeAck = false;
    Generic_IMUcan.bitrate = GENERIC_IMU_CFG_CAN_BITRATE;
    Generic_IMUcan.second_timeout = GENERIC_IMU_CFG_CAN_TIMEOUT;
    Generic_IMUcan.microsecond_timeout = GENERIC_IMU_CFG_CAN_MS_TIMEOUT;
    Generic_IMUcan.xfer_us_delay = GENERIC_IMU_CFG_CAN_XFER_US;

    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
    HkTelemetryPkt.DeviceEnabled = GENERIC_IMU_DEVICE_DISABLED;

    Generic_IMUHK.DeviceCounter = 0;
    Generic_IMUHK.DeviceStatus = 0;

    status = can_init_dev(&Generic_IMUcan);

    if (status == OS_SUCCESS)
    {
        printf("CAN device 0x%02x configured with speed %d \n", Generic_IMUcan.handle, Generic_IMUcan.bitrate);
    }
    else
    {
        printf("I2C device 0x%02x failed to initialize! \n", Generic_IMUcan.handle);
        status = OS_ERROR;
    }

    can_close_device(&Generic_IMUcan);

    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

  }

  Generic_imu ::
    ~Generic_imu()
  {
     // Close the device 
    can_close_device(&Generic_IMUcan);

    nos_destroy_link();
  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

// GENERIC_IMU_RequestHK
void Generic_imu :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

  int32_t status = OS_SUCCESS;

  HkTelemetryPkt.CommandCount++;

  if(HkTelemetryPkt.DeviceEnabled == GENERIC_IMU_DEVICE_ENABLED){
    status = GENERIC_IMU_RequestHK(&Generic_IMUcan, &Generic_IMUHK);
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

  this->tlmWrite_ReportedComponentCount(Generic_IMUHK.DeviceCounter);
  this->tlmWrite_DeviceStatus(Generic_IMUHK.DeviceStatus);
  this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
  this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
  this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
  this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
  this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

  
  // Tell the fprime command system that we have completed the processing of the supplied command with OK status
  this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// GENERIC_IMU_RequestData
void Generic_imu :: REQUEST_DATA_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

  int32_t status = OS_SUCCESS;

  if(HkTelemetryPkt.DeviceEnabled == GENERIC_IMU_DEVICE_ENABLED)
  {  
    HkTelemetryPkt.CommandCount++;
    status = GENERIC_IMU_RequestData(&Generic_IMUcan, &Generic_IMUData);
    if (status == OS_SUCCESS)
    {
      HkTelemetryPkt.DeviceCount++;
      this->log_ACTIVITY_HI_TELEM("RequestData command success\n");
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("RequestData command failed!\n");
    }
  }
  else
  {
    HkTelemetryPkt.CommandErrorCount++;
  }

  this->tlmWrite_X_Axis_LinearAcc(Generic_IMUData.X_Data.LinearAcc);
  this->tlmWrite_X_Axis_AngularAcc(Generic_IMUData.X_Data.AngularAcc);
  this->tlmWrite_Y_Axis_LinearAcc(Generic_IMUData.Y_Data.LinearAcc);
  this->tlmWrite_Y_Axis_AngularAcc(Generic_IMUData.Y_Data.AngularAcc);
  this->tlmWrite_Z_Axis_LinearAcc(Generic_IMUData.Z_Data.LinearAcc);
  this->tlmWrite_Z_Axis_AngularAcc(Generic_IMUData.Z_Data.AngularAcc);

  this->tlmWrite_ReportedComponentCount(Generic_IMUHK.DeviceCounter);
  this->tlmWrite_DeviceStatus(Generic_IMUHK.DeviceStatus);
  this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
  this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
  this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
  this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);

  
  // Tell the fprime command system that we have completed the processing of the supplied command with OK status
  this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void Generic_imu :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
  uint32_t status = OS_SUCCESS;

  status = GENERIC_IMU_CommandDevice(&Generic_IMUcan, GENERIC_IMU_DEVICE_NOOP_CMD);

  HkTelemetryPkt.CommandCount++;
  this->log_ACTIVITY_HI_TELEM("NOOP SENT");
  this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
  this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

  // Tell the fprime command system that we have completed the processing of the supplied command with OK status
  this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void Generic_imu :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

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

void Generic_imu :: ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_IMU_DEVICE_DISABLED)
    {
      HkTelemetryPkt.CommandCount++;

      Generic_IMUcan.handle = GENERIC_IMU_CFG_HANDLE;
      Generic_IMUcan.isUp = CAN_INTERFACE_DOWN;
      Generic_IMUcan.loopback = false;
      Generic_IMUcan.listenOnly = false;
      Generic_IMUcan.tripleSampling = false;
      Generic_IMUcan.oneShot = false;
      Generic_IMUcan.berrReporting = false;
      Generic_IMUcan.fd = false;
      Generic_IMUcan.presumeAck = false;
      Generic_IMUcan.bitrate = GENERIC_IMU_CFG_CAN_BITRATE;
      Generic_IMUcan.second_timeout = GENERIC_IMU_CFG_CAN_TIMEOUT;
      Generic_IMUcan.microsecond_timeout = GENERIC_IMU_CFG_CAN_MS_TIMEOUT;
      Generic_IMUcan.xfer_us_delay = GENERIC_IMU_CFG_CAN_XFER_US;

      status = can_init_dev(&Generic_IMUcan);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = GENERIC_IMU_DEVICE_ENABLED;
        HkTelemetryPkt.DeviceCount++;
        this->log_ACTIVITY_HI_TELEM("Enable command success!");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Enable command failed to init CAN!");
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

  void Generic_imu :: DISABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    int32_t status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled == GENERIC_IMU_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;

      status = can_close_device(&Generic_IMUcan);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = GENERIC_IMU_DEVICE_DISABLED;
        HkTelemetryPkt.DeviceCount++;
        this->log_ACTIVITY_HI_TELEM("Disable command success!");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        this->log_ACTIVITY_HI_TELEM("Disable command failed to close CAN!");
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

  inline Generic_imu_ActiveState Generic_imu :: get_active_state(uint8_t DeviceEnabled)
  {
    Generic_imu_ActiveState state;

    if(DeviceEnabled == GENERIC_IMU_DEVICE_ENABLED)
    {
      state.e = Generic_imu_ActiveState::ENABLED;
    }
    else
    {
      state.e = Generic_imu_ActiveState::DISABLED;
    }

    return state;
  }


}

