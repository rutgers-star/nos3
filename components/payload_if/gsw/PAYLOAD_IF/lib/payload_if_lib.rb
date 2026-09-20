# Library for PAYLOAD_IF Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
PAYLOAD_IF_CMD_SLEEP = 0.25
PAYLOAD_IF_RESPONSE_TIMEOUT = 5
PAYLOAD_IF_TEST_LOOP_COUNT = 1
PAYLOAD_IF_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_payload_if_hk()
    cmd("PAYLOAD_IF PAYLOAD_IF_REQ_HK")
    wait_check_packet("PAYLOAD_IF", "PAYLOAD_IF_HK_TLM", 1, PAYLOAD_IF_RESPONSE_TIMEOUT)
    sleep(PAYLOAD_IF_CMD_SLEEP)
end

def get_payload_if_data()
    cmd("PAYLOAD_IF PAYLOAD_IF_REQ_DATA")
    wait_check_packet("PAYLOAD_IF", "PAYLOAD_IF_DATA_TLM", 1, PAYLOAD_IF_RESPONSE_TIMEOUT)
    sleep(PAYLOAD_IF_CMD_SLEEP)
end

def payload_if_cmd(*command)
    count = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_payload_if_hk()
    current = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_payload_if_hk()
        current = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_payload_if_hk()
            current = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT")
        end
    end
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM CMD_COUNT >= #{count}")
end

def enable_payload_if()
    # Send command
    payload_if_cmd("PAYLOAD_IF PAYLOAD_IF_ENABLE_CC")
    # Confirm
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_payload_if()
    # Send command
    payload_if_cmd("PAYLOAD_IF PAYLOAD_IF_DISABLE_CC")
    # Confirm
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_payload_if()
    get_payload_if_hk()
    state = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_payload_if()
    end
end

def confirm_payload_if_data()
    dev_cmd_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_ERR_COUNT")
    
    get_payload_if_data()
    # Note these checks assume default simulator configuration
    raw_x = tlm("PAYLOAD_IF PAYLOAD_IF_DATA_TLM RAW_PAYLOAD_IF_X")
    check("PAYLOAD_IF PAYLOAD_IF_DATA_TLM RAW_PAYLOAD_IF_Y >= #{raw_x*2}")
    check("PAYLOAD_IF PAYLOAD_IF_DATA_TLM RAW_PAYLOAD_IF_Z >= #{raw_x*3}")

    get_payload_if_hk()
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("PAYLOAD_IF PAYLOAD_IF_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_payload_if_data_loop()
    PAYLOAD_IF_DEVICE_LOOP_COUNT.times do |n|
        confirm_payload_if_data()
    end
end

#
# Simulator Functions
#
def payload_if_prepare_ast()
    # Get to known state
    safe_payload_if()

    # Enable
    enable_payload_if()

    # Confirm data
    confirm_payload_if_data_loop()
end

def payload_if_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE PAYLOAD_IF_SIM_ENABLE")
end

def payload_if_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE PAYLOAD_IF_SIM_DISABLE")
end

def payload_if_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE PAYLOAD_IF_SIM_SET_STATUS with STATUS #{status}")
end
