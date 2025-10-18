require 'cosmos'
require 'cosmos/script'
require "generic_radio_lib.rb"

##
## Enable / disable, control hardware communications
##
GENERIC_RADIO_TEST_LOOP_COUNT.times do |n|

  safe_GENERIC_RADIO()

  #radio does not have enable/disable functionality, so we just check the device is alive.
  confirm_GENERIC_RADIO_data_loop()
end