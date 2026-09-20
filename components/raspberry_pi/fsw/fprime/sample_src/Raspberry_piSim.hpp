// ======================================================================
// \title  Raspberry_piSim.hpp
// \author jstar
// \brief  hpp file for Raspberry_piSim component implementation class
// ======================================================================

#ifndef Components_Raspberry_piSim_HPP
#define Components_Raspberry_piSim_HPP

#include "raspberry_pi_src/Raspberry_piSimComponentAc.hpp"
#include "raspberry_pi_src/Raspberry_piSim_ActiveStateEnumAc.hpp"

extern "C"{
#include "raspberry_pi_device.h"
#include "libuart.h"
}
  

#define RASPBERRY_PI_DEVICE_DISABLED 0
#define RASPBERRY_PI_DEVICE_ENABLED  1

typedef struct
{
    uint8_t                     CommandErrorCount;
    uint8_t                     CommandCount;
    uint8_t                     DeviceErrorCount;
    uint8_t                     DeviceCount;
    uint8_t                     DeviceEnabled;
} __attribute__((packed)) RASPBERRY_PI_Hk_tlm_t;
#define RASPBERRY_PI_HK_TLM_LNGTH sizeof(RASPBERRY_PI_Hk_tlm_t)


namespace Components {

  class Raspberry_piSim :
    public Raspberry_piSimComponentBase
  {

    public:

    uart_info_t Raspberry_piUart; 
    RASPBERRY_PI_Device_HK_tlm_t Raspberry_piHK; 
    RASPBERRY_PI_Device_Data_tlm_t Raspberry_piData;
    int32_t status = OS_SUCCESS;

    RASPBERRY_PI_Hk_tlm_t HkTelemetryPkt;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Raspberry_piSim object
      Raspberry_piSim(
          const char* const compName //!< The component name
      );

      //! Destroy Raspberry_piSim object
      ~Raspberry_piSim();

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

       void RASPBERRY_PI_SEQ_cmdHandler(
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

      inline Raspberry_piSim_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
