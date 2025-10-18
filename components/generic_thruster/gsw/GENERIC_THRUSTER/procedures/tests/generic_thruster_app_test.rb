require 'cosmos'
require 'cosmos/script'
require "generic_thruster_lib.rb"

##
## This script tests the standard cFS component application functionality.
## Currently this includes: 
##   Housekeeping, request telemetry to be published on the software bus
##   NOOP, no operation but confirm correct counters increment
##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##   Invalid ground command, confirm bad lengths and codes are rejected
##

# Get to known state
safe_generic_thruster()

##
##   Housekeeping, request telemetry to be published on the software bus
##
GENERIC_THRUSTER_TEST_LOOP_COUNT.times do |n|
    get_generic_thruster_hk()
end


##
## NOOP, no operation but confirm correct counters increment
##
GENERIC_THRUSTER_TEST_LOOP_COUNT.times do |n|
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_NOOP_CC")
end


##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
GENERIC_THRUSTER_TEST_LOOP_COUNT.times do |n|
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_NOOP_CC")
    cmd("GENERIC_THRUSTER GENERIC_THRUSTER_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
    get_generic_thruster_hk()
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT == 0")
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT == 0")
end


##
##   Invalid ground command, confirm bad lengths and codes are rejected
##
GENERIC_THRUSTER_TEST_LOOP_COUNT.times do |n|
    # Bad length
    cmd_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_THRUSTER GENERIC_THRUSTER_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
    get_generic_thruster_hk()
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end

for n in 6..(5 + GENERIC_THRUSTER_TEST_LOOP_COUNT)
    # Bad command codes
    cmd_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_THRUSTER GENERIC_THRUSTER_NOOP_CC with CCSDS_FC #{n+1}")
    get_generic_thruster_hk()
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end


##
##   Device testing
##

## Prepare
#cmd("GENERIC_ADCS GENERIC_ADCS_SET_MODE_CC with GNC_MODE PASSIVE")
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_ENABLE_CC")
#
## Get baseline degrees per second (DPS) from SIM_42_TRUTH
#pre_x = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_X_DPS")
#pre_y = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_Y_DPS")
#pre_z = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_Z_DPS")
#
## +X rotation via thrusters 0 and 2
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 0, PERCENTAGE 100")
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 2, PERCENTAGE 100")
#sleep 15
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 0, PERCENTAGE 0")
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 2, PERCENTAGE 0")
#post_x = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_X_DPS")
#post_y = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_Y_DPS")
#post_z = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_Z_DPS")
#puts "pre_x = #{pre_x}   ;   post_x #{post_x}"
#puts "pre_y = #{pre_y}   ;   post_y #{post_y}"
#puts "pre_z = #{pre_z}   ;   post_z #{post_z}"
#
## +X rotation via thrusters 1 and 3
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 1, PERCENTAGE 100")
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 3, PERCENTAGE 100")
#sleep 15
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 1, PERCENTAGE 0")
#cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 3, PERCENTAGE 0")
#post_x = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_X_DPS")
#post_y = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_Y_DPS")
#post_z = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_Z_DPS")
#puts "pre_x = #{pre_x}   ;   post_x #{post_x}"
#puts "pre_y = #{pre_y}   ;   post_y #{post_y}"
#puts "pre_z = #{pre_z}   ;   post_z #{post_z}"
