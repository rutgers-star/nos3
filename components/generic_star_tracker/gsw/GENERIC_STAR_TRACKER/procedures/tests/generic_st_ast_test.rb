require 'cosmos'
require 'cosmos/script'
require "generic_st_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
GENERIC_STAR_TRACKER_TEST_LOOP_COUNT.times do |n|
    # Prepare
    generic_star_tracker_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ERR_COUNT")
    generic_star_tracker_sim_disable()
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_COUNT == #{dev_cmd_cnt}")
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    generic_star_tracker_sim_enable()
    confirm_generic_star_tracker_data_loop()
end

