require 'cosmos'
require 'cosmos/script'
require "payload_if_lib.rb"

##
## This script tests the standard cFS component application functionality.
## Currently this includes: 
##   Housekeeping, request telemetry to be published on the software bus
##   NOOP, no operation but confirm correct counters increment
##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##   Invalid ground command, confirm bad lengths and codes are rejected
##

# Get to known state
safe_payload_if()

##
##   Housekeeping, request telemetry to be published on the software bus
##
PAYLOAD_IF_TEST_LOOP_COUNT.times do |n|
    get_payload_if_hk()
end


##
## NOOP, no operation but confirm correct counters increment
##
PAYLOAD_IF_TEST_LOOP_COUNT.times do |n|
    payload_if_cmd("PAYLOAD_IF PAYLOAD_IF_NOOP_CC")
end


##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
PAYLOAD_IF_TEST_LOOP_COUNT.times do |n|
    payload_if_cmd("PAYLOAD_IF PAYLOAD_IF_NOOP_CC")
    cmd("PAYLOAD_IF PAYLOAD_IF_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT == 0")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT == 0")
end


##
##   Invalid ground command, confirm bad lengths and codes are rejected
##
PAYLOAD_IF_TEST_LOOP_COUNT.times do |n|
    # Bad length
    cmd_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT")
    cmd("PAYLOAD_IF PAYLOAD_IF_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end

for n in 6..(5 + PAYLOAD_IF_TEST_LOOP_COUNT)
    # Bad command codes
    cmd_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT")
    cmd("PAYLOAD_IF PAYLOAD_IF_NOOP_CC with CCSDS_FC #{n+1}")
    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end
