require 'cosmos'
require 'cosmos/script'
require "gps_lib.rb"

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##


##
## Hardware failure
##
NOVATEL_OEM615_TEST_LOOP_COUNT.times do |n|
    # Prepare
    gps_prepare_ast()

    # Disable sim and confirm device error counts increase
    dev_cmd_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_ERR_COUNT")
    gps_sim_disable()
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_COUNT == #{dev_cmd_cnt}")
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_ERR_COUNT >= #{dev_cmd_err_cnt}")

    # Enable sim and confirm return to nominal operation
    gps_sim_enable()
    confirm_gps_data_loop()
end
