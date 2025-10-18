require 'cosmos'
require 'cosmos/script'
require "tmp100_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##   Configuration, reconfigure tmp100 instrument register
##


##
## Enable / disable, control hardware communications
##
TMP100_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_tmp100()

    # Manually command to disable when already disabled
    cmd_cnt = tlm("TMP100 TMP100_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("TMP100 TMP100_HK_TLM CMD_ERR_COUNT")
    cmd("TMP100 TMP100_DISABLE_CC")
    get_tmp100_hk()
    check("TMP100 TMP100_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("TMP100 TMP100_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_tmp100()

    # Confirm device counters increment without errors
    confirm_tmp100_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("TMP100 TMP100_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("TMP100 TMP100_HK_TLM CMD_ERR_COUNT")
    cmd("TMP100 TMP100_ENABLE_CC")
    get_tmp100_hk()
    check("TMP100 TMP100_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("TMP100 TMP100_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_tmp100_data_loop()

    # Disable
    disable_tmp100()
end


##
##   Configuration, reconfigure tmp100 instrument register
##
TMP100_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_tmp100()

    # Confirm configuration command denied if disabled
    cmd_cnt = tlm("TMP100 TMP100_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("TMP100 TMP100_HK_TLM CMD_ERR_COUNT")
    cmd("TMP100 TMP100_CONFIG_CC with DEVICE_CONFIG 10")
    get_tmp100_hk()
    check("TMP100 TMP100_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("TMP100 TMP100_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
    
    # Enable
    enable_tmp100()

    # Set configuration
    tmp100_cmd("TMP100 TMP100_CONFIG_CC with DEVICE_CONFIG #{n+1}")
    check("TMP100 TMP100_HK_TLM DEVICE_CONFIG == #{n+1}")
end
