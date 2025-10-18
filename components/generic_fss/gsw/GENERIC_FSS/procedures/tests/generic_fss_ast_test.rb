require 'cosmos'
require 'cosmos/script'
require "generic_fss_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
GENERIC_FSS_TEST_LOOP_COUNT.times do |n|
    # Prepare
    fss_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
    fss_sim_disable()
    check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_COUNT == #{dev_cmd_cnt}")
    check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    fss_sim_enable()
    confirm_fss_data_loop()
end