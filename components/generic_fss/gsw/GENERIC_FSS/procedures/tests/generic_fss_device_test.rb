require 'cosmos'
require 'cosmos/script'
require "generic_fss_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##


##
## Enable / disable, control hardware communications
##
GENERIC_FSS_TEST_LOOP_COUNT.times do |n|
    safe_fss() # Get to known state

    # Manually command to disable when already disabled
    cmd_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_FSS GENERIC_FSS_DISABLE_CC")
    get_fss_hk()
    check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_fss()

    # Confirm device counters increment without errors
    confirm_fss_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_FSS GENERIC_FSS_ENABLE_CC")
    get_fss_hk()
    check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_fss_data_loop()

    # Disable
    disable_fss()
end

