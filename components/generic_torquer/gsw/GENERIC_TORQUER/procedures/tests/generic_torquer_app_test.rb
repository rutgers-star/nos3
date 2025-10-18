require 'cosmos'
require 'cosmos/script'
require "generic_torquer_lib.rb"

##
## This script tests the standard cFS component application functionality.
## Currently this includes: 
##   Housekeeping, request telemetry to be published on the software bus
##   NOOP, no operation but confirm correct counters increment
##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##   Invalid ground command, confirm bad lengths and codes are rejected
##


##
##   Housekeeping, request telemetry to be published on the software bus
##
GENERIC_TORQUER_TEST_LOOP_COUNT.times do |n|
    get_generic_torquer_hk()
end


##
## NOOP, no operation but confirm correct counters increment
##
GENERIC_TORQUER_TEST_LOOP_COUNT.times do |n|
    generic_torquer_cmd("GENERIC_TORQUER GENERIC_TORQUER_NOOP_CC")
end


##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
GENERIC_TORQUER_TEST_LOOP_COUNT.times do |n|
    generic_torquer_cmd("GENERIC_TORQUER GENERIC_TORQUER_NOOP_CC")
    cmd("GENERIC_TORQUER GENERIC_TORQUER_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
    get_generic_torquer_hk()
    check("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_COUNT == 0")
    check("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_ERR_COUNT == 0")
end


##
##   Invalid ground command, confirm bad lengths and codes are rejected
##
GENERIC_TORQUER_TEST_LOOP_COUNT.times do |n|
    # Bad length
    cmd_cnt = tlm("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_ERR_COUNT")
    cmd("GENERIC_TORQUER GENERIC_TORQUER_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
    get_generic_torquer_hk()
    check("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end

for n in 6..(5 + GENERIC_TORQUER_TEST_LOOP_COUNT)
    # Bad command codes
    cmd_cnt = tlm("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_ERR_COUNT")
    cmd("GENERIC_TORQUER GENERIC_TORQUER_NOOP_CC with CCSDS_FC #{n+1}")
    get_generic_torquer_hk()
    check("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end
