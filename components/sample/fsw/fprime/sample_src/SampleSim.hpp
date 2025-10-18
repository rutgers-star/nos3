// ======================================================================
// \title  SampleSim.hpp
// \author jstar
// \brief  hpp file for SampleSim component implementation class
// ======================================================================

#ifndef Components_SampleSim_HPP
#define Components_SampleSim_HPP

#include "sample_src/SampleSimComponentAc.hpp"
#include "sample_src/SampleSim_ActiveStateEnumAc.hpp"

extern "C"{
#include "sample_device.h"
#include "libuart.h"
}
  

#define SAMPLE_DEVICE_DISABLED 0
#define SAMPLE_DEVICE_ENABLED  1

typedef struct
{
    uint8_t                     CommandErrorCount;
    uint8_t                     CommandCount;
    uint8_t                     DeviceErrorCount;
    uint8_t                     DeviceCount;
    uint8_t                     DeviceEnabled;
} __attribute__((packed)) SAMPLE_Hk_tlm_t;
#define SAMPLE_HK_TLM_LNGTH sizeof(SAMPLE_Hk_tlm_t)


namespace Components {

  class SampleSim :
    public SampleSimComponentBase
  {

    public:

    uart_info_t SampleUart; 
    SAMPLE_Device_HK_tlm_t SampleHK; 
    SAMPLE_Device_Data_tlm_t SampleData;
    int32_t status = OS_SUCCESS;

    SAMPLE_Hk_tlm_t HkTelemetryPkt;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct SampleSim object
      SampleSim(
          const char* const compName //!< The component name
      );

      //! Destroy SampleSim object
      ~SampleSim();

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

       void SAMPLE_SEQ_cmdHandler(
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

      inline SampleSim_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
