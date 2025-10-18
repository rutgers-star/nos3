require 'cosmos'
require 'cosmos/script'
require "raspberry_pi_lib.rb"

##
## This script tests the standard cFS component application functionality.
## Currently this includes: 
##   Housekeeping, request telemetry to be published on the software bus
##   NOOP, no operation but confirm correct counters increment
##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##   Invalid ground command, confirm bad lengths and codes are rejected
##

# Get to known state
safe_raspberry_pi()

##
##   Housekeeping, request telemetry to be published on the software bus
##
RASPBERRY_PI_TEST_LOOP_COUNT.times do |n|
    get_raspberry_pi_hk()
end


##
## NOOP, no operation but confirm correct counters increment
##
RASPBERRY_PI_TEST_LOOP_COUNT.times do |n|
    raspberry_pi_cmd("RASPBERRY_PI RASPBERRY_PI_NOOP_CC")
end


##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
RASPBERRY_PI_TEST_LOOP_COUNT.times do |n|
    raspberry_pi_cmd("RASPBERRY_PI RASPBERRY_PI_NOOP_CC")
    cmd("RASPBERRY_PI RASPBERRY_PI_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT == 0")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT == 0")
end


##
##   Invalid ground command, confirm bad lengths and codes are rejected
##
RASPBERRY_PI_TEST_LOOP_COUNT.times do |n|
    # Bad length
    cmd_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT")
    cmd("RASPBERRY_PI RASPBERRY_PI_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end

for n in 6..(5 + RASPBERRY_PI_TEST_LOOP_COUNT)
    # Bad command codes
    cmd_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT")
    cmd("RASPBERRY_PI RASPBERRY_PI_NOOP_CC with CCSDS_FC #{n+1}")
    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end
