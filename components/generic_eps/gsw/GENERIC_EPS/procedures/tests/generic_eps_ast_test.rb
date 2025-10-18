require 'cosmos'
require 'cosmos/script'
require "generic_eps_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
GENERIC_EPS_TEST_LOOP_COUNT.times do |n|
    # Prepare
    eps_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("GENERIC_EPS GENERIC_EPS_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_EPS GENERIC_EPS_HK_TLM DEVICE_ERR_COUNT")
    eps_sim_disable()
    check("GENERIC_EPS GENERIC_EPS_HK_TLM DEVICE_COUNT == #{dev_cmd_cnt}")
    check("GENERIC_EPS GENERIC_EPS_HK_TLM DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    eps_sim_enable()
    confirm_eps_data_loop()
end

