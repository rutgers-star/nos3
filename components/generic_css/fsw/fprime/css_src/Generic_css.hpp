// ======================================================================
// \title  Generic_css.hpp
// \author jstar
// \brief  hpp file for Generic_css component implementation class
// ======================================================================

#ifndef Components_Generic_css_HPP
#define Components_Generic_css_HPP

#include "css_src/Generic_cssComponentAc.hpp"
#include "css_src/Generic_css_ActiveStateEnumAc.hpp"

extern "C"{
#include "generic_css_device.h"
#include "libi2c.h"
}

#include "nos_link.h"

typedef struct
{
    uint8_t                         DeviceCount;
    uint8_t                         DeviceErrorCount;
    uint8_t                         CommandErrorCount;
    uint8_t                         CommandCount;
    uint8_t                         DeviceEnabled;
} CSS_Hk_tlm_t;
#define CSS_HK_TLM_LNGTH sizeof(CSS_Hk_tlm_t)

#define GENERIC_CSS_DEVICE_DISABLED 0
#define GENERIC_CSS_DEVICE_ENABLED  1

namespace Components {

  class Generic_css :
    public Generic_cssComponentBase
  {

    public:

    i2c_bus_info_t Generic_CSSI2c;
    GENERIC_CSS_Device_Data_tlm_t Generic_CSSData;
    CSS_Hk_tlm_t HkTelemetryPkt;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_css object
      Generic_css(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_css object
      ~Generic_css();

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

      void NOOP_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void ENABLE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void DISABLE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void REQUEST_HOUSEKEEPING_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void RESET_COUNTERS_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void REQUEST_DATA_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      inline Generic_css_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
