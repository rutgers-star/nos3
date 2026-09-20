require 'cosmos'
require 'cosmos/script'
require "raspberry_pi_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
RASPBERRY_PI_TEST_LOOP_COUNT.times do |n|
    # Prepare
    raspberry_pi_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_ERR_COUNT")
    raspberry_pi_sim_disable()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_COUNT == #{dev_cmd_cnt}")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    raspberry_pi_sim_enable()
    confirm_raspberry_pi_data_loop()
end


##
## Hardware status reporting fault
##
RASPBERRY_PI_TEST_LOOP_COUNT.times do |n|
    # Prepare
    raspberry_pi_prepare_ast()

    # Add a fault to status in the simulator
    raspberry_pi_sim_set_status(255)

    # Confirm that status register and that app disabled itself
    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_STATUS == 255")
    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_ENABLED == 'DISABLED'")
    
    # Clear simulator status fault
    raspberry_pi_sim_set_status(0)
end
