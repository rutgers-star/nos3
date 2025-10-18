// ======================================================================
// \title  Generic_star_tracker.hpp
// \author jstar
// \brief  hpp file for Generic_star_tracker component implementation class
// ======================================================================

#ifndef Components_Generic_star_tracker_HPP
#define Components_Generic_star_tracker_HPP

#include "st_src/Generic_star_trackerComponentAc.hpp"
#include "st_src/Generic_star_tracker_ActiveStateEnumAc.hpp"

extern "C"{
#include "generic_star_tracker_device.h"
#include "libuart.h"
}

typedef struct
{
    uint8_t                         DeviceCount;
    uint8_t                         DeviceErrorCount;
    uint8_t                         CommandErrorCount;
    uint8_t                         CommandCount;
    uint8_t                         DeviceEnabled;
} ST_Hk_tlm_t;
#define ST_HK_TLM_LNGTH sizeof(ST_Hk_tlm_t)

#define GENERIC_ST_DEVICE_DISABLED 0
#define GENERIC_ST_DEVICE_ENABLED  1

namespace Components {

  class Generic_star_tracker :
    public Generic_star_trackerComponentBase
  {

    public:

    uart_info_t Generic_star_trackerUart;
    GENERIC_STAR_TRACKER_Device_HK_tlm_t Generic_star_trackerHK;
    GENERIC_STAR_TRACKER_Device_Data_tlm_t Generic_star_trackerData;
    ST_Hk_tlm_t HkTelemetryPkt;
      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_star_tracker object
      Generic_star_tracker(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_star_tracker object
      ~Generic_star_tracker();

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
      ) override;

      void NOOP_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      void REQUEST_DATA_cmdHandler (
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

      inline Generic_star_tracker_ActiveState get_active_state(uint8_t DeviceEnabled);
  };

}

#endif
