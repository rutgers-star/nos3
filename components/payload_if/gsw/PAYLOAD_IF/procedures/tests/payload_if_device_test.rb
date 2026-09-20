require 'cosmos'
require 'cosmos/script'
require "payload_if_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##   Configuration, reconfigure payload_if instrument register
##


##
## Enable / disable, control hardware communications
##
PAYLOAD_IF_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_payload_if()

    # Manually command to disable when already disabled
    cmd_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT")
    cmd("PAYLOAD_IF PAYLOAD_IF_DISABLE_CC")
    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_payload_if()

    # Confirm device counters increment without errors
    confirm_payload_if_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT")
    cmd("PAYLOAD_IF PAYLOAD_IF_ENABLE_CC")
    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_payload_if_data_loop()

    # Disable
    disable_payload_if()
end


##
##   Configuration, reconfigure payload_if instrument register
##
PAYLOAD_IF_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_payload_if()

    # Confirm configuration command denied if disabled
    cmd_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT")
    cmd("PAYLOAD_IF PAYLOAD_IF_CONFIG_CC with DEVICE_CONFIG 10")
    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
    
    # Enable
    enable_payload_if()

    # Set configuration
    payload_if_cmd("PAYLOAD_IF PAYLOAD_IF_CONFIG_CC with DEVICE_CONFIG #{n+1}")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_CONFIG == #{n+1}")
end
