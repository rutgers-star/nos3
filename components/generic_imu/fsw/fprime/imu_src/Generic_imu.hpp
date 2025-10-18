// ======================================================================
// \title  Generic_imu.hpp
// \author jstar
// \brief  hpp file for Generic_imu component implementation class
// ======================================================================

#ifndef Components_Generic_imu_HPP
#define Components_Generic_imu_HPP

#include "imu_src/Generic_imuComponentAc.hpp"
#include "imu_src/Generic_imu_ActiveStateEnumAc.hpp"

extern "C"{
#include "generic_imu_device.h"
#include "generic_imu_platform_cfg.h"
#include "libcan.h"
}

#include "nos_link.h"

typedef struct
{
    uint8_t                         DeviceCount;
    uint8_t                         DeviceErrorCount;
    uint8_t                         CommandErrorCount;
    uint8_t                         CommandCount;
    uint8_t                         DeviceEnabled;
} IMU_Hk_tlm_t;
#define IMU_HK_TLM_LNGTH sizeof(IMU_Hk_tlm_t)

#define GENERIC_IMU_DEVICE_DISABLED 0
#define GENERIC_IMU_DEVICE_ENABLED  1

namespace Components {

  class Generic_imu :
    public Generic_imuComponentBase
  {

    public:

    can_info_t Generic_IMUcan;
    GENERIC_IMU_Device_HK_tlm_t Generic_IMUHK;
    GENERIC_IMU_Device_Data_tlm_t Generic_IMUData;
    IMU_Hk_tlm_t HkTelemetryPkt;


      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_imu object
      Generic_imu(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_imu object
      ~Generic_imu();

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

      void REQUEST_DATA_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

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

      inline Generic_imu_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
