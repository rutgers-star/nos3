require 'cosmos'
require 'cosmos/script'
require "sample_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##   Configuration, reconfigure sample instrument register
##


##
## Enable / disable, control hardware communications
##
SAMPLE_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_sample()

    # Manually command to disable when already disabled
    cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT")
    cmd("SAMPLE SAMPLE_DISABLE_CC")
    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_sample()

    # Confirm device counters increment without errors
    confirm_sample_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT")
    cmd("SAMPLE SAMPLE_ENABLE_CC")
    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_sample_data_loop()

    # Disable
    disable_sample()
end


##
##   Configuration, reconfigure sample instrument register
##
SAMPLE_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_sample()

    # Confirm configuration command denied if disabled
    cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT")
    cmd("SAMPLE SAMPLE_CONFIG_CC with DEVICE_CONFIG 10")
    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
    
    # Enable
    enable_sample()

    # Set configuration
    sample_cmd("SAMPLE SAMPLE_CONFIG_CC with DEVICE_CONFIG #{n+1}")
    check("SAMPLE SAMPLE_HK_TLM DEVICE_CONFIG == #{n+1}")
end
