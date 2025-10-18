require 'cosmos'
require 'cosmos/script'
require "sample_lib.rb"

##
## This script tests the standard cFS component application functionality.
## Currently this includes: 
##   Housekeeping, request telemetry to be published on the software bus
##   NOOP, no operation but confirm correct counters increment
##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##   Invalid ground command, confirm bad lengths and codes are rejected
##

# Get to known state
safe_sample()

##
##   Housekeeping, request telemetry to be published on the software bus
##
SAMPLE_TEST_LOOP_COUNT.times do |n|
    get_sample_hk()
end


##
## NOOP, no operation but confirm correct counters increment
##
SAMPLE_TEST_LOOP_COUNT.times do |n|
    sample_cmd("SAMPLE SAMPLE_NOOP_CC")
end


##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
SAMPLE_TEST_LOOP_COUNT.times do |n|
    sample_cmd("SAMPLE SAMPLE_NOOP_CC")
    cmd("SAMPLE SAMPLE_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM CMD_COUNT == 0")
    check("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == 0")
end


##
##   Invalid ground command, confirm bad lengths and codes are rejected
##
SAMPLE_TEST_LOOP_COUNT.times do |n|
    # Bad length
    cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT")
    cmd("SAMPLE SAMPLE_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end

for n in 6..(5 + SAMPLE_TEST_LOOP_COUNT)
    # Bad command codes
    cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT")
    cmd("SAMPLE SAMPLE_NOOP_CC with CCSDS_FC #{n+1}")
    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end
