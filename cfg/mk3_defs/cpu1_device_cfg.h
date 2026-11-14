#include "../nos3_defs/cpu1_device_cfg.h"

/*
** PAYLOAD_IF Configuration
** The payload link uses the FPGA-side RS-422 port (PL RS422-1, J14) at the
** ICD rate. Linux only creates /dev/ttyS3 for it once GomSpace's
** pl-rs422.service has loaded the bitstream and its device-tree overlay, so
** cFS must start after that service. On Linux the handle is the file
** descriptor, set when the port opens.
*/
#define PAYLOAD_IF_CFG
#define PAYLOAD_IF_CFG_STRING      "/dev/ttyS3"
#define PAYLOAD_IF_CFG_HANDLE      0
#define PAYLOAD_IF_CFG_BAUDRATE_HZ 115200
#define PAYLOAD_IF_CFG_MS_TIMEOUT  50 /* Max 255 */

#define PAYLOAD_IF_RX_TASK_NAME          "PAYLOAD_IF_RX"
#define PAYLOAD_IF_RX_TASK_STACK_SIZE    2048
#define PAYLOAD_IF_RX_TASK_PRIORITY      80
#define PAYLOAD_IF_RX_TASK_MS_DELAY      10
#define PAYLOAD_IF_RX_EXIT_SEM_NAME      "PAYLOAD_IF_RXSEM"
#define PAYLOAD_IF_RX_TASK_STOP_MS       1000

#define PAYLOAD_IF_HK_PERIOD_SEC_DEFAULT 5
#define PAYLOAD_IF_HK_PERIOD_SEC_MAX     3600
//#define PAYLOAD_IF_CFG_DEBUG
