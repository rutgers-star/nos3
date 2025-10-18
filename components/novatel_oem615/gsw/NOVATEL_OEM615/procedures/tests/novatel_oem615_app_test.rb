require 'cosmos'
require 'cosmos/script'
require "gps_lib.rb"

##
## This script tests the standard cFS component application functionality.
## Currently this includes: 
##   Housekeeping, request telemetry to be published on the software bus
##   NOOP, no operation but confirm correct counters increment
##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##   Invalid ground command, confirm bad lengths and codes are rejected
##

# Get to known state
safe_gps()

##
##   Housekeeping, request telemetry to be published on the software bus
##
NOVATEL_OEM615_TEST_LOOP_COUNT.times do |n|
    get_gps_hk()
end


##
## NOOP, no operation but confirm correct counters increment
##
NOVATEL_OEM615_TEST_LOOP_COUNT.times do |n|
    gps_cmd("NOVATEL_OEM615 NOVATEL_OEM615_NOOP_CC")
end


##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
NOVATEL_OEM615_TEST_LOOP_COUNT.times do |n|
    gps_cmd("NOVATEL_OEM615 NOVATEL_OEM615_NOOP_CC")
    cmd("NOVATEL_OEM615 NOVATEL_OEM615_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
    get_gps_hk()
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT == 0")
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT == 0")
end


##
##   Invalid ground command, confirm bad lengths and codes are rejected
##
NOVATEL_OEM615_TEST_LOOP_COUNT.times do |n|
    # Bad length
    cmd_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT")
    cmd("NOVATEL_OEM615 NOVATEL_OEM615_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
    get_gps_hk()
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end

for n in 8..(7 + NOVATEL_OEM615_TEST_LOOP_COUNT)
    # Bad command codes
    cmd_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT")
    cmd("NOVATEL_OEM615 NOVATEL_OEM615_NOOP_CC with CCSDS_FC #{n+1}")
    get_gps_hk()
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end
