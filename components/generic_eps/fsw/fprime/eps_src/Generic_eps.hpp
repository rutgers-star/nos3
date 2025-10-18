// ======================================================================
// \title  Generic_eps.hpp
// \author jstar
// \brief  hpp file for Generic_eps component implementation class
// ======================================================================

#ifndef Components_Generic_eps_HPP
#define Components_Generic_eps_HPP

#include "eps_src/Generic_epsComponentAc.hpp"
#include "eps_src/Generic_eps_StateEnumAc.hpp"
#include "eps_src/Generic_eps_FlagEnumAc.hpp"
#include "eps_src/Generic_eps_SW_NUMEnumAc.hpp"

extern "C"{
  #include "generic_eps_device.h"
  #include "libi2c.h"
  }

typedef struct
{
    uint8_t                     DeviceCount;
    uint8_t                     DeviceErrorCount;
    uint8_t                     CommandErrorCount;
    uint8_t                     CommandCount;
} __attribute__((packed)) EPS_Hk_tlm_t;
#define EPS_HK_TLM_LNGTH sizeof(EPS_Hk_tlm_t)

namespace Components {

  class Generic_eps :
    public Generic_epsComponentBase
  {

    public:

      i2c_bus_info_t Generic_epsI2c;
      GENERIC_EPS_Device_HK_tlm_t Generic_epsHK;
      EPS_Hk_tlm_t HkTelemetryPkt;
      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_eps object
      Generic_eps(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_eps object
      ~Generic_eps();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------

      void NOOP_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void RESET_COUNTERS_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void SWITCH_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq,
        Generic_eps_SW_NUM switch_num,
        Generic_eps_State switch_state
      ) override;

      void REQUEST_HOUSEKEEPING_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      void Update_Counters();
      void Update_Base_Tlm();
      void Update_SW_Tlm();

      inline Generic_eps_State get_switch_state(uint16_t switch_status);
      inline Generic_eps_Flag get_switch_flag(uint16_t switch_status);

  };

}

#endif
