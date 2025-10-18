require 'cosmos'
require 'cosmos/script'
require "generic_css_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##   Configuration, reconfigure generic_css instrument register
##


##
## Enable / disable, control hardware communications
##
GENERIC_CSS_TEST_LOOP_COUNT.times do |n|
    safe_generic_css() # Get to known state

    # Manually command to disable when already disabled
    cmd_cnt = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_CSS GENERIC_CSS_DISABLE_CC")
    get_generic_css_hk()
    check("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_generic_css()

    # Confirm device counters increment without errors
    confirm_generic_css_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_ERR_COUNT")
    cmd("GENERIC_CSS GENERIC_CSS_ENABLE_CC")
    get_generic_css_hk()
    check("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_generic_css_data_loop()

    # Disable
    disable_generic_css()
end