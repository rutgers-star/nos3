require 'cosmos'
require 'cosmos/script'
require "generic_torquer_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
GENERIC_TORQUER_TEST_LOOP_COUNT.times do |n|
    # Prepare
    generic_torquer_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T DEVICE_ERR_COUNT")
    generic_torquer_sim_disable()
    check("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T DEVICE_COUNT == #{dev_cmd_cnt}")
    check("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    generic_torquer_sim_enable()
    confirm_generic_torquer_data_loop()
end