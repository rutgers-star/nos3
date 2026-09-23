require 'cosmos'
require 'cosmos/script'
require "payload_if_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
PAYLOAD_IF_TEST_LOOP_COUNT.times do |n|
    # Prepare
    payload_if_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_ERR_COUNT")
    payload_if_sim_disable()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_COUNT == #{dev_cmd_cnt}")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    payload_if_sim_enable()
    confirm_payload_if_data_loop()
end


##
## Hardware status reporting fault
##
PAYLOAD_IF_TEST_LOOP_COUNT.times do |n|
    # Prepare
    payload_if_prepare_ast()

    # Add a fault to status in the simulator
    payload_if_sim_set_status(255)

    # Confirm that status register and that app disabled itself
    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_STATUS == 255")
    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_ENABLED == 'DISABLED'")
    
    # Clear simulator status fault
    payload_if_sim_set_status(0)
end
