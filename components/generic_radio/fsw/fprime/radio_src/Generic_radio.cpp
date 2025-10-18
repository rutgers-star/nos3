// ======================================================================
// \title  Generic_radio.cpp
// \author jstar
// \brief  cpp file for Generic_radio component implementation class
// ======================================================================

#include "radio_src/Generic_radio.hpp"
#include <Fw/Logger/Logger.hpp>
#include "FpConfig.hpp"




void init_socket_data() {

  
}

namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Generic_radio ::
    Generic_radio(const char* const compName) :
      Generic_radioComponentBase(compName)
  {
        int status = OS_SUCCESS;
        uint8_t run_status = OS_SUCCESS;

        /* Initialize HWLIB */
          #ifdef _NOS_ENGINE_LINK_
              nos_init_link();
          #endif

        /*
        ** Initialize sockets
        */
        RadioSocket.sockfd = -1;
        RadioSocket.port_num = GENERIC_RADIO_CFG_UDP_RADIO_TO_FSW;
        RadioSocket.ip_address = GENERIC_RADIO_CFG_FSW_IP;
        RadioSocket.address_family = ip_ver_4;
        RadioSocket.type = dgram;
        RadioSocket.category = client;
        RadioSocket.block = false;
        RadioSocket.keep_alive = false;
        RadioSocket.created = false;
        RadioSocket.bound = false;
        RadioSocket.listening = false;
        RadioSocket.connected = false;

        status = socket_create(&RadioSocket);
        
        if (status != SOCKET_SUCCESS)
        {
            printf("GENERIC_RADIO: Radio interface create error %d\n", status);
            run_status = OS_ERROR;
        }
        else
        {
            printf("GENERIC_RADIO: Radio Interface %d created successfully!\n", RadioSocket.sockfd);
        }

        ProxSocket.sockfd = -1;
        ProxSocket.port_num = GENERIC_RADIO_CFG_UDP_PROX_TO_FSW;
        ProxSocket.ip_address = GENERIC_RADIO_CFG_FSW_IP;
        ProxSocket.address_family = ip_ver_4;
        ProxSocket.type = dgram;
        ProxSocket.category = client;
        ProxSocket.block = false;
        ProxSocket.keep_alive = false;
        ProxSocket.created = false;
        ProxSocket.bound = false;
        ProxSocket.listening = false;
        ProxSocket.connected = false;

        status = socket_create(&ProxSocket);
        if (status != SOCKET_SUCCESS)
        {
            printf("GENERIC_RADIO: Proximity interface create error %d\n", status);
            run_status = OS_ERROR;
        }
        else
        {
            printf("GENERIC_RADIO: Proximity Interface %d created successfully!\n", ProxSocket.sockfd);
        }

        HkTelemetryPkt.CommandCount = 0;
        HkTelemetryPkt.CommandErrorCount = 0;
        HkTelemetryPkt.DeviceCount = 0;
        HkTelemetryPkt.DeviceErrorCount = 0;
  }

  Generic_radio ::
    ~Generic_radio()
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Generic_radio :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    HkTelemetryPkt.CommandCount++;

    this->log_ACTIVITY_HI_TELEM("NOOP command success!");
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_radio :: CONFIG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 config_value){
    int32_t status = OS_SUCCESS;
    HkTelemetryPkt.CommandCount++;

    status = GENERIC_RADIO_SetConfiguration(&RadioSocket, config_value);
    if(status == OS_SUCCESS)
    {
      HkTelemetryPkt.DeviceCount++;
      this->log_ACTIVITY_HI_TELEM("Config command successful!");
    }
    else
    {
      HkTelemetryPkt.DeviceErrorCount++;
      this->log_ACTIVITY_HI_TELEM("Config command failed!");
    }

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Generic_radio :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
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

  // GENERIC_radio_RequestHK
  void Generic_radio :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    int32_t status = OS_SUCCESS;

    status = GENERIC_RADIO_RequestHK(&RadioSocket, &RadioHK);
    
    if (status == OS_SUCCESS)
    {
        this->log_ACTIVITY_HI_TELEM("RequestHK command success\n");
    }
    else
    {
        this->log_ACTIVITY_HI_TELEM("RequestHK command failed!\n");
    }

    this->tlmWrite_ReportedComponentCount(RadioHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(RadioHK.DeviceConfig);
    this->tlmWrite_ProxySignal(RadioHK.ProxSignal);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    
    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

}
