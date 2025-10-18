require 'cosmos'
require 'cosmos/script'
require "generic_mag_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##   Configuration, reconfigure generic_mag instrument register
##


##
## Enable / disable, control hardware communications
##
GENERIC_MAG_TEST_LOOP_COUNT.times do |n|
    safe_generic_mag() # Get to known state

    # Manually command to disable when already disabled
    cmd_cnt = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_MAG GENERIC_MAG_DISABLE_CC")
    get_generic_mag_hk()
    check("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_generic_mag()

    # Confirm device counters increment without errors
    confirm_generic_mag_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_MAG GENERIC_MAG_ENABLE_CC")
    get_generic_mag_hk()
    check("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_generic_mag_data_loop()

    # Disable
    disable_generic_mag()
end