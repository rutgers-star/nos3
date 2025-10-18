require 'cosmos'
require 'cosmos/script'
require "generic_eps_lib.rb"

##
## This script tests the standard cFS component application functionality.
## Currently this includes: 
##   Housekeeping, request telemetry to be published on the software bus
##   NOOP, no operation but confirm correct counters increment
##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##   Invalid ground command, confirm bad lengths and codes are rejected
##

# Get to known state
safe_eps()

##
##   Housekeeping, request telemetry to be published on the software bus
##
GENERIC_EPS_TEST_LOOP_COUNT.times do |n|
    get_eps_hk()
end


##
## NOOP, no operation but confirm correct counters increment
##
GENERIC_EPS_TEST_LOOP_COUNT.times do |n|
    eps_cmd("GENERIC_EPS GENERIC_EPS_NOOP_CC")
end


##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
GENERIC_EPS_TEST_LOOP_COUNT.times do |n|
    eps_cmd("GENERIC_EPS GENERIC_EPS_NOOP_CC")
    cmd("GENERIC_EPS GENERIC_EPS_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
    get_eps_hk()
    check("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_COUNT == 0")
    check("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_ERR_COUNT == 0")
end


##
##   Invalid ground command, confirm bad lengths and codes are rejected
##
GENERIC_EPS_TEST_LOOP_COUNT.times do |n|
    # Bad length
    cmd_cnt = tlm("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_EPS GENERIC_EPS_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
    get_eps_hk()
    check("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end

for n in 6..(5 + GENERIC_EPS_TEST_LOOP_COUNT)
    # Bad command codes
    cmd_cnt = tlm("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_EPS GENERIC_EPS_NOOP_CC with CCSDS_FC #{n+1}")
    get_eps_hk()
    check("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_EPS GENERIC_EPS_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end
