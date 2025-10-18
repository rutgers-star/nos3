# Library for SAMPLE Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
SAMPLE_CMD_SLEEP = 0.25
SAMPLE_RESPONSE_TIMEOUT = 5
SAMPLE_TEST_LOOP_COUNT = 1
SAMPLE_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_sample_hk()
    cmd("SAMPLE SAMPLE_REQ_HK")
    wait_check_packet("SAMPLE", "SAMPLE_HK_TLM", 1, SAMPLE_RESPONSE_TIMEOUT)
    sleep(SAMPLE_CMD_SLEEP)
end

def get_sample_data()
    cmd("SAMPLE SAMPLE_REQ_DATA")
    wait_check_packet("SAMPLE", "SAMPLE_DATA_TLM", 1, SAMPLE_RESPONSE_TIMEOUT)
    sleep(SAMPLE_CMD_SLEEP)
end

def sample_cmd(*command)
    count = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_sample_hk()
    current = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_sample_hk()
        current = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_sample_hk()
            current = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
        end
    end
    check("SAMPLE SAMPLE_HK_TLM CMD_COUNT >= #{count}")
end

def enable_sample()
    # Send command
    sample_cmd("SAMPLE SAMPLE_ENABLE_CC")
    # Confirm
    check("SAMPLE SAMPLE_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_sample()
    # Send command
    sample_cmd("SAMPLE SAMPLE_DISABLE_CC")
    # Confirm
    check("SAMPLE SAMPLE_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_sample()
    get_sample_hk()
    state = tlm("SAMPLE SAMPLE_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_sample()
    end
end

def confirm_sample_data()
    dev_cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM DEVICE_ERR_COUNT")
    
    get_sample_data()
    # Note these checks assume default simulator configuration
    raw_x = tlm("SAMPLE SAMPLE_DATA_TLM RAW_SAMPLE_X")
    check("SAMPLE SAMPLE_DATA_TLM RAW_SAMPLE_Y >= #{raw_x*2}")
    check("SAMPLE SAMPLE_DATA_TLM RAW_SAMPLE_Z >= #{raw_x*3}")

    get_sample_hk()
    check("SAMPLE SAMPLE_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("SAMPLE SAMPLE_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_sample_data_loop()
    SAMPLE_DEVICE_LOOP_COUNT.times do |n|
        confirm_sample_data()
    end
end

#
# Simulator Functions
#
def sample_prepare_ast()
    # Get to known state
    safe_sample()

    # Enable
    enable_sample()

    # Confirm data
    confirm_sample_data_loop()
end

def sample_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_SIM_ENABLE")
end

def sample_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_SIM_DISABLE")
end

def sample_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_SIM_SET_STATUS with STATUS #{status}")
end
