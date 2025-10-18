require 'cosmos'
require 'cosmos/script'
require "arducam_lib.rb"

##
## This script tests the standard cFS component application functionality.
## Currently this includes: 
##   Housekeeping, request telemetry to be published on the software bus
##   NOOP, no operation but confirm correct counters increment
##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##   Invalid ground command, confirm bad lengths and codes are rejected
##

# Get to known state
safe_arducam()

##
##   Housekeeping, request telemetry to be published on the software bus
##
ARDUCAM_TEST_LOOP_COUNT.times do |n|
    get_arducam_hk()
end


##
## NOOP, no operation but confirm correct counters increment
##
ARDUCAM_TEST_LOOP_COUNT.times do |n|
    arducam_cmd("ARDUCAM CAM_NOOP_CC")
end


##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
ARDUCAM_TEST_LOOP_COUNT.times do |n|
    arducam_cmd("ARDUCAM CAM_NOOP_CC")
    cmd("ARDUCAM CAM_RESET_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
    get_arducam_hk()
    check("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT == 0")
    check("ARDUCAM ARDUCAM_HK_TLM_T COMMANDERRORCOUNT == 0")
end


##
##   Invalid ground command, confirm bad lengths and codes are rejected
##
ARDUCAM_TEST_LOOP_COUNT.times do |n|
    # Bad length
    cmd_cnt = tlm("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT")
    cmd_err_cnt = tlm("ARDUCAM ARDUCAM_HK_TLM_T COMMANDERRORCOUNT")
    cmd("ARDUCAM CAM_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
    get_arducam_hk()
    check("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT == #{cmd_cnt}")
    check("ARDUCAM ARDUCAM_HK_TLM_T COMMANDERRORCOUNT == #{cmd_err_cnt+1}")
end

for n in 6..(5 + ARDUCAM_TEST_LOOP_COUNT)
    # Bad command codes
    cmd_cnt = tlm("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT")
    cmd_err_cnt = tlm("ARDUCAM ARDUCAM_HK_TLM_T COMMANDERRORCOUNT")
    cmd("ARDUCAM CAM_NOOP_CC with CCSDS_FC #{n+1}")
    get_arducam_hk()
    check("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT == #{cmd_cnt}")
    check("ARDUCAM ARDUCAM_HK_TLM_T COMMANDERRORCOUNT == #{cmd_err_cnt+1}")
end
