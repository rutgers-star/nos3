require 'cosmos'
require 'cosmos/script'
require "raspberry_pi_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##   Configuration, reconfigure raspberry_pi instrument register
##


##
## Enable / disable, control hardware communications
##
RASPBERRY_PI_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_raspberry_pi()

    # Manually command to disable when already disabled
    cmd_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT")
    cmd("RASPBERRY_PI RASPBERRY_PI_DISABLE_CC")
    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_raspberry_pi()

    # Confirm device counters increment without errors
    confirm_raspberry_pi_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT")
    cmd("RASPBERRY_PI RASPBERRY_PI_ENABLE_CC")
    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_raspberry_pi_data_loop()

    # Disable
    disable_raspberry_pi()
end


##
##   Configuration, reconfigure raspberry_pi instrument register
##
RASPBERRY_PI_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_raspberry_pi()

    # Confirm configuration command denied if disabled
    cmd_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT")
    cmd("RASPBERRY_PI RASPBERRY_PI_CONFIG_CC with DEVICE_CONFIG 10")
    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
    
    # Enable
    enable_raspberry_pi()

    # Set configuration
    raspberry_pi_cmd("RASPBERRY_PI RASPBERRY_PI_CONFIG_CC with DEVICE_CONFIG #{n+1}")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_CONFIG == #{n+1}")
end
