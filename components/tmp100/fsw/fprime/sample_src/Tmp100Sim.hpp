// ======================================================================
// \title  Tmp100Sim.hpp
// \author jstar
// \brief  hpp file for Tmp100Sim component implementation class
// ======================================================================

#ifndef Components_Tmp100Sim_HPP
#define Components_Tmp100Sim_HPP

#include "tmp100_src/Tmp100SimComponentAc.hpp"
#include "tmp100_src/Tmp100Sim_ActiveStateEnumAc.hpp"

extern "C"{
#include "tmp100_device.h"
#include "libuart.h"
}
  

#define TMP100_DEVICE_DISABLED 0
#define TMP100_DEVICE_ENABLED  1

typedef struct
{
    uint8_t                     CommandErrorCount;
    uint8_t                     CommandCount;
    uint8_t                     DeviceErrorCount;
    uint8_t                     DeviceCount;
    uint8_t                     DeviceEnabled;
} __attribute__((packed)) TMP100_Hk_tlm_t;
#define TMP100_HK_TLM_LNGTH sizeof(TMP100_Hk_tlm_t)


namespace Components {

  class Tmp100Sim :
    public Tmp100SimComponentBase
  {

    public:

    uart_info_t Tmp100Uart; 
    TMP100_Device_HK_tlm_t Tmp100HK; 
    TMP100_Device_Data_tlm_t Tmp100Data;
    int32_t status = OS_SUCCESS;

    TMP100_Hk_tlm_t HkTelemetryPkt;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Tmp100Sim object
      Tmp100Sim(
          const char* const compName //!< The component name
      );

      //! Destroy Tmp100Sim object
      ~Tmp100Sim();

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

       void TMP100_SEQ_cmdHandler(
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

      inline Tmp100Sim_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
