require 'cosmos'
require 'cosmos/script'
require "sample_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
SAMPLE_TEST_LOOP_COUNT.times do |n|
    # Prepare
    sample_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM DEVICE_ERR_COUNT")
    sample_sim_disable()
    check("SAMPLE SAMPLE_HK_TLM DEVICE_COUNT == #{dev_cmd_cnt}")
    check("SAMPLE SAMPLE_HK_TLM DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    sample_sim_enable()
    confirm_sample_data_loop()
end


##
## Hardware status reporting fault
##
SAMPLE_TEST_LOOP_COUNT.times do |n|
    # Prepare
    sample_prepare_ast()

    # Add a fault to status in the simulator
    sample_sim_set_status(255)

    # Confirm that status register and that app disabled itself
    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM DEVICE_STATUS == 255")
    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM DEVICE_ENABLED == 'DISABLED'")
    
    # Clear simulator status fault
    sample_sim_set_status(0)
end
