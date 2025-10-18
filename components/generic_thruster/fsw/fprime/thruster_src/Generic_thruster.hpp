// ======================================================================
// \title  Generic_thruster.hpp
// \author jstar
// \brief  hpp file for Generic_thruster component implementation class
// ======================================================================

#ifndef Components_Generic_thruster_HPP
#define Components_Generic_thruster_HPP

#include "thruster_src/Generic_thrusterComponentAc.hpp"
#include "thruster_src/Generic_thruster_ActiveStateEnumAc.hpp"
#include "thruster_src/Generic_thruster_thrusterNumsEnumAc.hpp"


extern "C"{
#include "generic_thruster_device.h"
#include "libuart.h"
}

#include "nos_link.h"

#define THRUSTER_NUM 4

typedef struct
{
    uint8_t                         DeviceCount;
    uint8_t                         DeviceErrorCount;
    uint8_t                         CommandErrorCount;
    uint8_t                         CommandCount;
    uint8_t                         DeviceEnabled;
    uint8_t                         Percentage[THRUSTER_NUM];
} THRUSTER_Hk_tlm_t;
#define THRUSTER_HK_TLM_LNGTH sizeof(THRUSTER_Hk_tlm_t)

#define GENERIC_THRUSTER_DEVICE_DISABLED 0
#define GENERIC_THRUSTER_DEVICE_ENABLED  1


namespace Components {
  
  class Generic_thruster :
  public Generic_thrusterComponentBase
  {
    
    public:
    
    uart_info_t ThrusterUart;
    THRUSTER_Hk_tlm_t HkTelemetryPkt;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_thruster object
      Generic_thruster(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_thruster object
      ~Generic_thruster();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------

      void NOOP_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      void ENABLE_cmdHandler (
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void DISABLE_cmdHandler (
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void RESET_COUNTERS_cmdHandler (
        FwOpcodeType opCode,
        U32 cmdSeq
      ) override;

      void REQUEST_HOUSEKEEPING_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      void SET_PERCENTAGE_cmdHandler(
          FwOpcodeType opCode, //!< The opcode
          U32 cmdSeq, //!< The command sequence number
          const U8 percent, //!< Percentage to set Thruster
          const Generic_thruster_thrusterNums thruster_number //!< Thruster Number to set
      ) override;

      inline Generic_thruster_ActiveState get_active_state(uint8_t DeviceEnabled); 

  };

}

#endif
