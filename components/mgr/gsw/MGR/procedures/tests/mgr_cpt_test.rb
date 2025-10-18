require 'cosmos'
require 'cosmos/script'
require "cfs_lib.rb"

##
## NOOP
##
initial_command_count = tlm("MGR MGR_HK_TLM CMD_COUNT")
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_NOOP_CC")
wait_check("MGR MGR_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Successful Enable
##
initial_command_count = tlm("MGR MGR_HK_TLM CMD_COUNT")
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_ENABLE_CC")
wait_check("MGR MGR_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ENABLED == 'ENABLED'", 30)

sleep(5)

##
## Failed Enable (doubled)
##
initial_command_count = tlm("MGR MGR_HK_TLM CMD_COUNT")
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_ENABLE_CC")
wait_check("MGR MGR_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT > #{initial_device_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ENABLED == 'ENABLED'", 30)

sleep(5)

##
## Device Config
##
initial_command_count = tlm("MGR MGR_HK_TLM CMD_COUNT")
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
configure_value = ask "Enter Configuration Value (integer between 0 and 4,294,967,295):"
cmd("MGR MGR_CONFIG_CC with DEVICE_CONFIG #{configure_value}")
wait_check("MGR MGR_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Housekeeping w/ Device (and verify config)
##
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_REQ_HK")
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_CONFIG == #{configure_value}", 30)

sleep(5)

##
## Data w/ Device
##
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_REQ_DATA")
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Successful Disable
##
initial_command_count = tlm("MGR MGR_HK_TLM CMD_COUNT")
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_DISABLE_CC")
wait_check("MGR MGR_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ENABLED == 'DISABLED'", 30)

sleep(5)

##
## Failed Disable (doubled)
##
initial_command_count = tlm("MGR MGR_HK_TLM CMD_COUNT")
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_DISABLE_CC")
wait_check("MGR MGR_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT > #{initial_device_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ENABLED == 'DISABLED'", 30)

sleep(5)

##
## HK without Device
##
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_REQ_HK")
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Data without Device
##
initial_error_count = tlm("MGR MGR_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("MGR MGR_HK_TLM DEVICE_ERR_COUNT")
cmd("MGR MGR_REQ_DATA")
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Reset Counters
##
cmd("MGR MGR_RST_COUNTERS_CC")
wait_check("MGR MGR_HK_TLM CMD_COUNT == 0", 30)
wait_check("MGR MGR_HK_TLM CMD_ERR_COUNT == 0", 30)
wait_check("MGR MGR_HK_TLM DEVICE_COUNT == 0", 30)
wait_check("MGR MGR_HK_TLM DEVICE_ERR_COUNT == 0", 30)