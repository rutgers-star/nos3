// ======================================================================
// \title  Generic_mag.hpp
// \author jstar
// \brief  hpp file for Generic_mag component implementation class
// ======================================================================

#ifndef Components_Generic_mag_HPP
#define Components_Generic_mag_HPP

#include "mag_src/Generic_magComponentAc.hpp"
#include "mag_src/Generic_mag_ActiveStateEnumAc.hpp"

extern "C"{
#include "generic_mag_device.h"
#include "libspi.h"
}
  
#include "nos_link.h"

typedef struct
{
    uint8_t                         DeviceCount;
    uint8_t                         DeviceErrorCount;
    uint8_t                         CommandErrorCount;
    uint8_t                         CommandCount;
    uint8_t                         DeviceEnabled;
} MAG_Hk_tlm_t;
#define MAG_HK_TLM_LNGTH sizeof(MAG_Hk_tlm_t)

#define GENERIC_MAG_DEVICE_DISABLED 0
#define GENERIC_MAG_DEVICE_ENABLED  1

namespace Components {

  class Generic_mag :
    public Generic_magComponentBase
  {

    public:

    spi_info_t Generic_magSpi;
    GENERIC_MAG_Device_Data_tlm_t Generic_magData;
    MAG_Hk_tlm_t HkTelemetryPkt;
      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_mag object
      Generic_mag(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_mag object
      ~Generic_mag();

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

      void RESET_COUNTERS_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

       void REQUEST_DATA_cmdHandler(
          FwOpcodeType opCode, //!< The opcode
          U32 cmdSeq //!< The command sequence number
      ) override;

      void REQUEST_HOUSEKEEPING_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      inline Generic_mag_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
