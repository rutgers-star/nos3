# Library for ARDUCAM Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
ARDUCAM_CMD_SLEEP = 0.25
ARDUCAM_RESPONSE_TIMEOUT = 5
ARDUCAM_TEST_LOOP_COUNT = 1
ARDUCAM_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_arducam_hk()
    cmd("ARDUCAM CAM_SEND_HK_CC")
    wait_check_packet("ARDUCAM", "ARDUCAM_HK_TLM_T", 1, ARDUCAM_RESPONSE_TIMEOUT)
    sleep(ARDUCAM_CMD_SLEEP)
end

def arducam_cmd(*command)
    count = tlm("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_arducam_hk()
    current = tlm("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_arducam_hk()
        current = tlm("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_arducam_hk()
            current = tlm("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT")
        end
    end
    check("ARDUCAM ARDUCAM_HK_TLM_T COMMANDCOUNT >= #{count}")
end

def safe_arducam()
    get_arducam_hk()
end

def confirm_arducam_data()
    dev_cmd_cnt = tlm("ARDUCAM ARDUCAM_HK_TLM_T DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("ARDUCAM ARDUCAM_HK_TLM_T DEVICE_ERR_COUNT")
    cmd("ARDUCAM CAM_EXP3_CC")
    get_arducam_hk()
    # Note these checks assume default simulator configuration
    check("ARDUCAM EXP_TLM_T CAM_FIFO_LENGTH >= 0")
    check("ARDUCAM EXP_TLM_T CAM_FIFO_DATA != NULL")
    get_arducam_hk()
    check("ARDUCAM ARDUCAM_HK_TLM_T DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("ARDUCAM ARDUCAM_HK_TLM_T DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_arducam_data_loop()
    ARDUCAM_DEVICE_LOOP_COUNT.times do |n|
        confirm_arducam_data()
    end
end

#
# Simulator Functions
#
def arducam_prepare_ast()
    # Get to known state
    safe_arducam()

    # Confirm data
    confirm_arducam_data_loop()
end

def arducam_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE ARDUCAM_SIM_SET_STATUS with STATUS #{status}")
end
