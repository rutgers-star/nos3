// ======================================================================
// \title  Generic_radio.hpp
// \author jstar
// \brief  hpp file for Generic_radio component implementation class
// ======================================================================

#ifndef Components_Generic_radio_HPP
#define Components_Generic_radio_HPP

#include "radio_src/Generic_radioComponentAc.hpp"


extern "C"{
#include "generic_radio_device.h"
}

typedef struct
{
    uint8_t                         DeviceCount;
    uint8_t                         DeviceErrorCount;
    uint8_t                         CommandErrorCount;
    uint8_t                         CommandCount;
} RADIO_Hk_tlm_t;
#define RADIO_HK_TLM_LNGTH sizeof(RADIO_Hk_tlm_t)

namespace Components {

  class Generic_radio :
    public Generic_radioComponentBase
  {

    public:


    socket_info_t RadioSocket;
    socket_info_t ProxSocket;
    GENERIC_RADIO_Device_HK_tlm_t RadioHK;
    RADIO_Hk_tlm_t HkTelemetryPkt;
    uint8_t RadioData;
    uint16_t SCID = 0x42;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_radio object
      Generic_radio(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_radio object
      ~Generic_radio();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------

      //! Handler implementation for command TODO
      //!
      //! TODO
      // void TODO_cmdHandler(
      //     FwOpcodeType opCode, //!< The opcode
      //     U32 cmdSeq //!< The command sequence number
      // ) override;

      void REQUEST_HOUSEKEEPING_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      );

      void NOOP_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      void CONFIG_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq,
        U32 config_value
      ) override;

      void RESET_COUNTERS_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

  };

}

#endif
