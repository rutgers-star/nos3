require 'cosmos'
require 'cosmos/script'
require "generic_thruster_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##   Configuration, reconfigure generic_thruster instrument register
##


##
## Enable / disable, control hardware communications
##
GENERIC_THRUSTER_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_generic_thruster()

    # Manually command to disable when already disabled
    cmd_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_THRUSTER GENERIC_THRUSTER_DISABLE_CC")
    get_generic_thruster_hk()
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_generic_thruster()

    # Confirm device counters increment without errors
    confirm_generic_thruster_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_THRUSTER GENERIC_THRUSTER_ENABLE_CC")
    get_generic_thruster_hk()
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_generic_thruster_data_loop()

    # Disable
    disable_generic_thruster()
end


##
##   Configuration, reconfigure generic_thruster instrument register
##
GENERIC_THRUSTER_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_generic_thruster()

    # Confirm configuration command denied if disabled
    cmd_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 0, PERCENTAGE 0")
    get_generic_thruster_hk()
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
    
    # Enable
    enable_generic_thruster()

    # Set configuration
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 0, PERCENTAGE #{n+1}")
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 1, PERCENTAGE #{n+1}")
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 2, PERCENTAGE #{n+1}")
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 3, PERCENTAGE #{n+1}")

    #check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM DEVICE_CONFIG == #{n+1}")
end
