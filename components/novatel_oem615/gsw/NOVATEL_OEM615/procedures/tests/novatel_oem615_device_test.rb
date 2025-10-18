require 'cosmos'
require 'cosmos/script'
require "gps_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##


##
## Enable / disable, control hardware communications
##
NOVATEL_OEM615_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_gps()

    # Manually command to disable when already disabled
    cmd_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT")
    cmd("NOVATEL_OEM615 NOVATEL_OEM615_DISABLE_CC")
    get_gps_hk()
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_gps()

    # Confirm device counters increment without errors
    confirm_gps_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT")
    cmd("NOVATEL_OEM615 NOVATEL_OEM615_ENABLE_CC")
    get_gps_hk()
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_gps_data_loop()

    # Disable
    disable_gps()
end