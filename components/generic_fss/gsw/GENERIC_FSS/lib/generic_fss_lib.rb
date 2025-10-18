# Library for GENERIC_FSS Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
GENERIC_FSS_CMD_SLEEP = 0.25
GENERIC_FSS_RESPONSE_TIMEOUT = 5
GENERIC_FSS_TEST_LOOP_COUNT = 1
GENERIC_FSS_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_fss_hk()
    cmd("GENERIC_FSS GENERIC_FSS_REQ_HK")
    wait_check_packet("GENERIC_FSS", "GENERIC_FSS_HK_TLM", 1, GENERIC_FSS_RESPONSE_TIMEOUT)
    sleep(GENERIC_FSS_CMD_SLEEP)
end

def get_fss_data()
    cmd("GENERIC_FSS GENERIC_FSS_REQ_DATA")
    wait_check_packet("GENERIC_FSS", "GENERIC_FSS_DATA_TLM", 1, GENERIC_FSS_RESPONSE_TIMEOUT)
    sleep(GENERIC_FSS_CMD_SLEEP)
end

def fss_cmd(*command)
    count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_fss_hk()
    current = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_fss_hk()
        current = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_fss_hk()
            current = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
        end
    end
    check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT >= #{count}")
end

def enable_fss()
    # Send command
    fss_cmd("GENERIC_FSS GENERIC_FSS_ENABLE_CC")
    # Confirm
    check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_fss()
    # Send command
    fss_cmd("GENERIC_FSS GENERIC_FSS_DISABLE_CC")
    # Confirm
    check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_fss()
    get_fss_hk()
    state = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_fss()
    end
end

def confirm_fss_data()
    dev_cmd_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
    
    get_fss_data()
    svb0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_0")
    svb1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_1")
    svb2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_2")

    fss_error = tlm("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_ERROR_CODE")

    truth_42_alpha = -Math.atan2(svb2, svb0)
    truth_42_beta = Math.atan2(svb1, svb0)

    diff = 0.03

    if fss_error == 0
        wait_check_tolerance("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_ALPHA", truth_42_alpha, diff, GENERIC_FSS_RESPONSE_TIMEOUT)

        wait_check_tolerance("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_BETA", truth_42_beta, diff, GENERIC_FSS_RESPONSE_TIMEOUT)
    end

    get_fss_hk()
    check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_fss_data_loop()
    GENERIC_FSS_DEVICE_LOOP_COUNT.times do |n|
        confirm_fss_data()
    end
end

#
# Simulator Functions
#
def fss_prepare_ast()
    # Get to known state
    safe_fss()

    # Enable
    enable_fss()

    # Confirm data
    confirm_fss_data_loop()
end

def fss_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE GENERIC_FSS_SIM_ENABLE")
end

def fss_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE GENERIC_FSS_SIM_DISABLE")
end

def fss_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE GENERIC_FSS_SIM_SET_STATUS with STATUS #{status}")
end
