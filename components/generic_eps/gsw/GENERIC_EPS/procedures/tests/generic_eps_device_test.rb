require 'cosmos'
require 'cosmos/script'
require "generic_eps_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##


##
## Enable / disable, control hardware communications
##
GENERIC_EPS_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_eps()

    # Confirm device counters increment without errors
    confirm_eps_data_loop()

end

