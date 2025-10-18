require 'cosmos'
require 'cosmos/script'
require "generic_radio_lib.rb"


safe_GENERIC_RADIO()

##
##   Housekeeping, request telemetry to be published on the software bus
##
GENERIC_RADIO_TEST_LOOP_COUNT.times do |n|
  get_GENERIC_RADIO_hk()
end

##
## NOOP, no operation but confirm correct counters increment
##
GENERIC_RADIO_TEST_LOOP_COUNT.times do |n|
  GENERIC_RADIO_cmd("GENERIC_RADIO GENERIC_RADIO_NOOP_CC")
end

##
## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
##
GENERIC_RADIO_TEST_LOOP_COUNT.times do |n|
  GENERIC_RADIO_cmd("GENERIC_RADIO GENERIC_RADIO_NOOP_CC")
  cmd("GENERIC_RADIO GENERIC_RADIO_RST_COUNTERS_CC")
  get_GENERIC_RADIO_hk()
  check("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_COUNT == 0")
  check("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_ERR_COUNT == 0")
end

##
## injecting bad commmands, checking error counters increase
##
GENERIC_RADIO_TEST_LOOP_COUNT.times do |n|
   # Bad length
   cmd_cnt = tlm("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_COUNT")
   cmd_err_cnt = tlm("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_ERR_COUNT")
   cmd("GENERIC_RADIO GENERIC_RADIO_NOOP_CC with CCSDS_LENGTH #{n+2}") # Note +2 due to CCSDS already being +1
   get_GENERIC_RADIO_hk()
   check("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_COUNT == #{cmd_cnt}")
   check("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end

for n in 6..(5 + GENERIC_RADIO_TEST_LOOP_COUNT)
  # Bad command codes
  cmd_cnt = tlm("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_COUNT")
  cmd_err_cnt = tlm("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_ERR_COUNT")
  cmd("GENERIC_RADIO GENERIC_RADIO_NOOP_CC with CCSDS_FC #{n+1}")
  get_GENERIC_RADIO_hk()
  check("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_COUNT == #{cmd_cnt}")
  check("GENERIC_RADIO GENERIC_RADIO_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
end