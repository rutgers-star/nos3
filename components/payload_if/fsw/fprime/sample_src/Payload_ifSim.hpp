// ======================================================================
// \title  Payload_ifSim.hpp
// \author jstar
// \brief  hpp file for Payload_ifSim component implementation class
// ======================================================================

#ifndef Components_Payload_ifSim_HPP
#define Components_Payload_ifSim_HPP

#include "payload_if_src/Payload_ifSimComponentAc.hpp"
#include "payload_if_src/Payload_ifSim_ActiveStateEnumAc.hpp"

extern "C"{
#include "payload_if_device.h"
#include "libuart.h"
}
  

#define PAYLOAD_IF_DEVICE_DISABLED 0
#define PAYLOAD_IF_DEVICE_ENABLED  1

typedef struct
{
    uint8_t                     CommandErrorCount;
    uint8_t                     CommandCount;
    uint8_t                     DeviceErrorCount;
    uint8_t                     DeviceCount;
    uint8_t                     DeviceEnabled;
} __attribute__((packed)) PAYLOAD_IF_Hk_tlm_t;
#define PAYLOAD_IF_HK_TLM_LNGTH sizeof(PAYLOAD_IF_Hk_tlm_t)


namespace Components {

  class Payload_ifSim :
    public Payload_ifSimComponentBase
  {

    public:

    uart_info_t Payload_ifUart; 
    PAYLOAD_IF_Device_HK_tlm_t Payload_ifHK; 
    PAYLOAD_IF_Device_Data_tlm_t Payload_ifData;
    int32_t status = OS_SUCCESS;

    PAYLOAD_IF_Hk_tlm_t HkTelemetryPkt;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Payload_ifSim object
      Payload_ifSim(
          const char* const compName //!< The component name
      );

      //! Destroy Payload_ifSim object
      ~Payload_ifSim();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------


      void REQUEST_HOUSEKEEPING_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      void NOOP_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      )override;

       void PAYLOAD_IF_SEQ_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      )override;

      void ENABLE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      )override;

      void DISABLE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      )override;

      void RESET_COUNTERS_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      )override;

      void CONFIGURE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq,
        const U32 config
      )override;

      inline Payload_ifSim_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
