require 'cosmos'
require 'cosmos/script'
require "generic_radio_lib.rb"


##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##


##
## Hardware failure
##
GENERIC_RADIO_TEST_LOOP_COUNT.times do |n|
  generic_radio_prepare_ast()

  # Disable sim and confirm device error counts increase
  get_GENERIC_RADIO_hk()
  dev_cmd_cnt = tlm("GENERIC_RADIO GENERIC_RADIO_HK_TLM DEVICE_COUNTER")
  dev_cmd_err_cnt = tlm("GENERIC_RADIO GENERIC_RADIO_HK_TLM DEVICE_ERR_COUNT")

  generic_radio_sim_disable()
  
  cmd("GENERIC_RADIO GENERIC_RADIO_NOOP_CC") #sending command that should fail due to sim disabled

  get_GENERIC_RADIO_hk()

  check("GENERIC_RADIO GENERIC_RADIO_HK_TLM DEVICE_COUNTER >= #{dev_cmd_cnt}")
  check("GENERIC_RADIO GENERIC_RADIO_HK_TLM DEVICE_ERR_COUNT > #{dev_cmd_err_cnt}")

  # Enable sim and confirm return to nominal operation
  generic_radio_sim_enable()
  get_GENERIC_RADIO_hk();
  confirm_GENERIC_RADIO_data_loop()
end