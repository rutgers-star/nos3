# Library for RASPBERRY_PI Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
RASPBERRY_PI_CMD_SLEEP = 0.25
RASPBERRY_PI_RESPONSE_TIMEOUT = 5
RASPBERRY_PI_TEST_LOOP_COUNT = 1
RASPBERRY_PI_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_raspberry_pi_hk()
    cmd("RASPBERRY_PI RASPBERRY_PI_REQ_HK")
    wait_check_packet("RASPBERRY_PI", "RASPBERRY_PI_HK_TLM", 1, RASPBERRY_PI_RESPONSE_TIMEOUT)
    sleep(RASPBERRY_PI_CMD_SLEEP)
end

def get_raspberry_pi_data()
    cmd("RASPBERRY_PI RASPBERRY_PI_REQ_DATA")
    wait_check_packet("RASPBERRY_PI", "RASPBERRY_PI_DATA_TLM", 1, RASPBERRY_PI_RESPONSE_TIMEOUT)
    sleep(RASPBERRY_PI_CMD_SLEEP)
end

def raspberry_pi_cmd(*command)
    count = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_raspberry_pi_hk()
    current = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_raspberry_pi_hk()
        current = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_raspberry_pi_hk()
            current = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT")
        end
    end
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM CMD_COUNT >= #{count}")
end

def enable_raspberry_pi()
    # Send command
    raspberry_pi_cmd("RASPBERRY_PI RASPBERRY_PI_ENABLE_CC")
    # Confirm
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_raspberry_pi()
    # Send command
    raspberry_pi_cmd("RASPBERRY_PI RASPBERRY_PI_DISABLE_CC")
    # Confirm
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_raspberry_pi()
    get_raspberry_pi_hk()
    state = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_raspberry_pi()
    end
end

def confirm_raspberry_pi_data()
    dev_cmd_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_ERR_COUNT")
    
    get_raspberry_pi_data()
    # Note these checks assume default simulator configuration
    raw_x = tlm("RASPBERRY_PI RASPBERRY_PI_DATA_TLM RAW_RASPBERRY_PI_X")
    check("RASPBERRY_PI RASPBERRY_PI_DATA_TLM RAW_RASPBERRY_PI_Y >= #{raw_x*2}")
    check("RASPBERRY_PI RASPBERRY_PI_DATA_TLM RAW_RASPBERRY_PI_Z >= #{raw_x*3}")

    get_raspberry_pi_hk()
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("RASPBERRY_PI RASPBERRY_PI_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_raspberry_pi_data_loop()
    RASPBERRY_PI_DEVICE_LOOP_COUNT.times do |n|
        confirm_raspberry_pi_data()
    end
end

#
# Simulator Functions
#
def raspberry_pi_prepare_ast()
    # Get to known state
    safe_raspberry_pi()

    # Enable
    enable_raspberry_pi()

    # Confirm data
    confirm_raspberry_pi_data_loop()
end

def raspberry_pi_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE RASPBERRY_PI_SIM_ENABLE")
end

def raspberry_pi_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE RASPBERRY_PI_SIM_DISABLE")
end

def raspberry_pi_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE RASPBERRY_PI_SIM_SET_STATUS with STATUS #{status}")
end
