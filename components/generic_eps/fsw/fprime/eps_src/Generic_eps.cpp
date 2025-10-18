// ======================================================================
// \title  Generic_eps.cpp
// \author jstar
// \brief  cpp file for Generic_eps component implementation class
// ======================================================================

#include "eps_src/Generic_eps.hpp"
#include "FpConfig.hpp"
  
#include "nos_link.h"

namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Generic_eps ::
    Generic_eps(const char* const compName) :
      Generic_epsComponentBase(compName)
  {

    /* Initialize HWLIB */
    nos_init_link();
    
    int32_t status = OS_SUCCESS;
     /* Open device specific protocols */
    Generic_epsI2c.handle = GENERIC_EPS_CFG_I2C_HANDLE;
    Generic_epsI2c.addr = GENERIC_EPS_CFG_I2C_ADDRESS;
    Generic_epsI2c.isOpen = I2C_CLOSED;
    Generic_epsI2c.speed = GENERIC_EPS_CFG_I2C_SPEED;
    status = i2c_master_init(&Generic_epsI2c);

    if (status == OS_SUCCESS)
    {
        printf("I2C device 0x%02x configured with speed %d \n", Generic_epsI2c.addr, Generic_epsI2c.speed);
    }
    else
    {
        printf("I2C device 0x%02x failed to initialize! \n", Generic_epsI2c.addr);
        status = OS_ERROR;
    }

  }

  Generic_eps ::
    ~Generic_eps()
  {

    i2c_master_close(&Generic_epsI2c);

    nos_destroy_link();


  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Generic_eps :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    HkTelemetryPkt.CommandCount++;

    this->log_ACTIVITY_HI_TELEM("NOOP command success\n");
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_eps :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;

    this->log_ACTIVITY_HI_TELEM("Reset Counters command success\n");

    Update_Counters();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_eps :: SWITCH_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Generic_eps_SW_NUM switch_num, Generic_eps_State switch_state) {

    int32_t status = OS_SUCCESS;
    uint8_t value;

    if(switch_state.e == Generic_eps_State::ON)
    {
      value = 0xAA;
    }
    else
    {
      value = 0x00;
    }

    if(switch_num < 8)
    {
      HkTelemetryPkt.CommandCount++;
      status = GENERIC_EPS_CommandSwitch(&Generic_epsI2c, switch_num.e, value, &Generic_epsHK);

      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        this->log_ACTIVITY_HI_TELEM("Switch command success\n");
      }
      else
      {
        this->log_ACTIVITY_HI_TELEM("Switch command failed\n");
        HkTelemetryPkt.DeviceErrorCount++;
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Switch command failed\n");
    }

    Update_Counters();
    Update_SW_Tlm();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);

  }

  // GENERIC_EPS_RequestHK
  void Generic_eps :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    int32_t status = OS_SUCCESS;
    
    status = GENERIC_EPS_RequestHK(&Generic_epsI2c, &Generic_epsHK);
    if (status == OS_SUCCESS)
    {
        this->log_ACTIVITY_HI_TELEM("RequestHK command success\n");
        HkTelemetryPkt.CommandCount++;
        HkTelemetryPkt.DeviceCount++;
    }
    else
    {
        this->log_ACTIVITY_HI_TELEM("RequestHK command failed!\n");
        HkTelemetryPkt.CommandErrorCount++;
        HkTelemetryPkt.DeviceErrorCount++;
    }

    Update_Counters();
    Update_Base_Tlm();
    sleep(1);
    Update_SW_Tlm();
    sleep(1); // ensure all telemetry goes thru

    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_eps :: Update_Counters(){

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    
  }

  void Generic_eps :: Update_Base_Tlm(){

    this->tlmWrite_BatteryVoltage(Generic_epsHK.BatteryVoltage / 1000.0);
    this->tlmWrite_BatteryTemperature((Generic_epsHK.BatteryTemperature / 100.0) - 60);
    this->tlmWrite_Bus3p3Voltage(Generic_epsHK.Bus3p3Voltage / 1000.0);
    this->tlmWrite_Bus5p0Voltage(Generic_epsHK.Bus5p0Voltage / 1000.0);
    this->tlmWrite_Bus12Voltage(Generic_epsHK.Bus12Voltage / 1000.0);
    this->tlmWrite_EPSTemperature((Generic_epsHK.EPSTemperature / 100.0) - 60);
    this->tlmWrite_SolarArrayVoltage(Generic_epsHK.SolarArrayVoltage / 1000.0);
    this->tlmWrite_SolarArrayTemperature(Generic_epsHK.SolarArrayTemperature / 1000.0);
    
    this->tlmWrite_RawBatteryVoltage(Generic_epsHK.BatteryVoltage);
    this->tlmWrite_RawBatteryTemperature(Generic_epsHK.BatteryTemperature);
    this->tlmWrite_RawBus3p3V(Generic_epsHK.Bus3p3Voltage);
    this->tlmWrite_RawBus5p0V(Generic_epsHK.Bus5p0Voltage);
    this->tlmWrite_RawBus12V(Generic_epsHK.Bus12Voltage);
    this->tlmWrite_RawEPSTemperature(Generic_epsHK.EPSTemperature);
    this->tlmWrite_RawSAVoltage(Generic_epsHK.SolarArrayVoltage);
    this->tlmWrite_RawSATemperature(Generic_epsHK.SolarArrayTemperature);

  }

  void Generic_eps :: Update_SW_Tlm() {

    this->tlmWrite_RawSW0Voltage(Generic_epsHK.Switch[0].Voltage);
    this->tlmWrite_RawSW0Current(Generic_epsHK.Switch[0].Current);
    this->tlmWrite_SW0Voltage(Generic_epsHK.Switch[0].Voltage / 1000.0);
    this->tlmWrite_SW0Current(Generic_epsHK.Switch[0].Current / 1000.0);
    this->tlmWrite_SW0State(get_switch_state(Generic_epsHK.Switch[0].Status));
    this->tlmWrite_SW0Flag(get_switch_flag(Generic_epsHK.Switch[0].Status));

    this->tlmWrite_RawSW1Voltage(Generic_epsHK.Switch[1].Voltage);
    this->tlmWrite_RawSW1Current(Generic_epsHK.Switch[1].Current);
    this->tlmWrite_SW1Voltage(Generic_epsHK.Switch[1].Voltage / 1000.0);
    this->tlmWrite_SW1Current(Generic_epsHK.Switch[1].Current / 1000.0);
    this->tlmWrite_SW1State(get_switch_state(Generic_epsHK.Switch[1].Status));
    this->tlmWrite_SW1Flag(get_switch_flag(Generic_epsHK.Switch[1].Status));

    this->tlmWrite_RawSW2Voltage(Generic_epsHK.Switch[2].Voltage);
    this->tlmWrite_RawSW2Current(Generic_epsHK.Switch[2].Current);
    this->tlmWrite_SW2Voltage(Generic_epsHK.Switch[2].Voltage / 1000.0);
    this->tlmWrite_SW2Current(Generic_epsHK.Switch[2].Current / 1000.0);
    this->tlmWrite_SW2State(get_switch_state(Generic_epsHK.Switch[2].Status));
    this->tlmWrite_SW2Flag(get_switch_flag(Generic_epsHK.Switch[2].Status));

    this->tlmWrite_RawSW3Voltage(Generic_epsHK.Switch[3].Voltage);
    this->tlmWrite_RawSW3Current(Generic_epsHK.Switch[3].Current);
    this->tlmWrite_SW3Voltage(Generic_epsHK.Switch[3].Voltage / 1000.0);
    this->tlmWrite_SW3Current(Generic_epsHK.Switch[3].Current / 1000.0);
    this->tlmWrite_SW3State(get_switch_state(Generic_epsHK.Switch[3].Status));
    this->tlmWrite_SW3Flag(get_switch_flag(Generic_epsHK.Switch[3].Status));

    this->tlmWrite_RawSW4Voltage(Generic_epsHK.Switch[4].Voltage);
    this->tlmWrite_RawSW4Current(Generic_epsHK.Switch[4].Current);
    this->tlmWrite_SW4Voltage(Generic_epsHK.Switch[4].Voltage / 1000.0);
    this->tlmWrite_SW4Current(Generic_epsHK.Switch[4].Current / 1000.0);
    this->tlmWrite_SW4State(get_switch_state(Generic_epsHK.Switch[4].Status));
    this->tlmWrite_SW4Flag(get_switch_flag(Generic_epsHK.Switch[4].Status));

    this->tlmWrite_RawSW5Voltage(Generic_epsHK.Switch[5].Voltage);
    this->tlmWrite_RawSW5Current(Generic_epsHK.Switch[5].Current);
    this->tlmWrite_SW5Voltage(Generic_epsHK.Switch[5].Voltage / 1000.0);
    this->tlmWrite_SW5Current(Generic_epsHK.Switch[5].Current / 1000.0);
    this->tlmWrite_SW5State(get_switch_state(Generic_epsHK.Switch[5].Status));
    this->tlmWrite_SW5Flag(get_switch_flag(Generic_epsHK.Switch[5].Status));
    
    this->tlmWrite_RawSW6Voltage(Generic_epsHK.Switch[6].Voltage);
    this->tlmWrite_RawSW6Current(Generic_epsHK.Switch[6].Current);
    this->tlmWrite_SW6Voltage(Generic_epsHK.Switch[6].Voltage / 1000.0);
    this->tlmWrite_SW6Current(Generic_epsHK.Switch[6].Current / 1000.0);
    this->tlmWrite_SW6State(get_switch_state(Generic_epsHK.Switch[6].Status));
    this->tlmWrite_SW6Flag(get_switch_flag(Generic_epsHK.Switch[6].Status));
    sleep(1);
    
    this->tlmWrite_RawSW7Voltage(Generic_epsHK.Switch[7].Voltage);
    this->tlmWrite_RawSW7Current(Generic_epsHK.Switch[7].Current);
    this->tlmWrite_SW7Voltage(Generic_epsHK.Switch[7].Voltage / 1000.0);
    this->tlmWrite_SW7Current(Generic_epsHK.Switch[7].Current / 1000.0);
    this->tlmWrite_SW7State(get_switch_state(Generic_epsHK.Switch[7].Status));
    this->tlmWrite_SW7Flag(get_switch_flag(Generic_epsHK.Switch[7].Status));

  }

  inline Generic_eps_State Generic_eps :: get_switch_state(uint16_t switch_status)
  {
    Generic_eps_State sw;
    if((switch_status & 0x00FF) == 0xAA){
      sw.e = Generic_eps_State::ON;
    }else{
      sw.e = Generic_eps_State::OFF;
    }

    return sw;
  }

  inline Generic_eps_Flag Generic_eps :: get_switch_flag(uint16_t switch_status)
  {
    Generic_eps_Flag sw;
    if(((switch_status & 0xFF00) >> 8) == 0x00)
    {
      sw.e = Generic_eps_Flag::HEALTHY;
    }
    else{
      sw.e = Generic_eps_Flag::UNHEALTHY_PLACEHOLDER;
    }
    // implement other flags

    return sw;
  }

}