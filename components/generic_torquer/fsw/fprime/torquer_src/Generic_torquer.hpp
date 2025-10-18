// ======================================================================
// \title  Generic_torquer.hpp
// \author jstar
// \brief  hpp file for Generic_torquer component implementation class
// ======================================================================

#ifndef Components_Generic_torquer_HPP
#define Components_Generic_torquer_HPP

#include "torquer_src/Generic_torquerComponentAc.hpp"
#include "torquer_src/Generic_torquer_tq_numEnumAc.hpp"
#include "torquer_src/Generic_torquer_ActiveStateEnumAc.hpp"

extern "C"{
#include "generic_torquer_device.h"
#include "libtrq.h"
}

#include "nos_link.h"

typedef struct
{
    uint8_t                         DeviceCount;
    uint8_t                         DeviceErrorCount;
    uint8_t                         CommandErrorCount;
    uint8_t                         CommandCount;
    uint8_t                         DeviceEnabled;
    trq_info_t                      trqDevice[3];
    GENERIC_TORQUER_Device_tlm_t    trqHk[3];
} TORQUER_Hk_tlm_t;
#define TORQUER_HK_TLM_LNGTH sizeof(TORQUER_Hk_tlm_t)

#define GENERIC_TORQUER_DEVICE_DISABLED 0
#define GENERIC_TORQUER_DEVICE_ENABLED  1

namespace Components {

  class Generic_torquer :
    public Generic_torquerComponentBase
  {

    public:

      TORQUER_Hk_tlm_t HkTelemetryPkt;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_torquer object
      Generic_torquer(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_torquer object
      ~Generic_torquer();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------

      //! Handler implementation for command TODO
      //!
      //! TODO
      /*void TODO_cmdHandler(
          FwOpcodeType opCode, //!< The opcode
          U32 cmdSeq //!< The command sequence number
      ) override;*/

      void NOOP_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      );

      void RESET_COUNTERS_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      );

      void ENABLE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      );

      void DISABLE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      );

      void REQUEST_HOUSEKEEPING_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      );

      void ALL_CONFIG_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq,
        U8 Percent_0,
        U8 Direction_0,
        U8 Percent_1,
        U8 Direction_1,
        U8 Percent_2,
        U8 Direction_2
      );

      void GENERIC_TORQUER_CONFIG_cmdHandler(
          FwOpcodeType opCode, //!< The opcode
          U32 cmdSeq, //!< The command sequence number
          Generic_torquer_tq_num torquerNum,
          U8 Percent, //!< Greeting to repeat in the Hello event
          U8 Direction

      ) ;

      inline Generic_torquer_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
