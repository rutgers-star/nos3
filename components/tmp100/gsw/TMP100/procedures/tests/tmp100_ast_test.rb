require 'cosmos'
require 'cosmos/script'
require "tmp100_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
TMP100_TEST_LOOP_COUNT.times do |n|
    # Prepare
    tmp100_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("TMP100 TMP100_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("TMP100 TMP100_HK_TLM DEVICE_ERR_COUNT")
    tmp100_sim_disable()
    check("TMP100 TMP100_HK_TLM DEVICE_COUNT == #{dev_cmd_cnt}")
    check("TMP100 TMP100_HK_TLM DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    tmp100_sim_enable()
    confirm_tmp100_data_loop()
end


##
## Hardware status reporting fault
##
TMP100_TEST_LOOP_COUNT.times do |n|
    # Prepare
    tmp100_prepare_ast()

    # Add a fault to status in the simulator
    tmp100_sim_set_status(255)

    # Confirm that status register and that app disabled itself
    get_tmp100_hk()
    check("TMP100 TMP100_HK_TLM DEVICE_STATUS == 255")
    get_tmp100_hk()
    check("TMP100 TMP100_HK_TLM DEVICE_ENABLED == 'DISABLED'")
    
    # Clear simulator status fault
    tmp100_sim_set_status(0)
end
