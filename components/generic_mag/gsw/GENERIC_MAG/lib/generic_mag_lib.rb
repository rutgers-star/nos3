# Library for GENERIC_MAG Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
GENERIC_MAG_CMD_SLEEP = 0.25
GENERIC_MAG_RESPONSE_TIMEOUT = 5
GENERIC_MAG_TEST_LOOP_COUNT = 1
GENERIC_MAG_DEVICE_LOOP_COUNT = 5
GENERIC_MAG_DEVICE_NT_DIFFERENCE = 1000

#
# Functions
#
def get_generic_mag_hk()
    cmd("GENERIC_MAG GENERIC_MAG_REQ_HK")
    wait_check_packet("GENERIC_MAG", "GENERIC_MAG_HK_TLM", 1, GENERIC_MAG_RESPONSE_TIMEOUT)
    sleep(GENERIC_MAG_CMD_SLEEP)
end

def get_generic_mag_data()
    cmd("GENERIC_MAG GENERIC_MAG_REQ_DATA")
    wait_check_packet("GENERIC_MAG", "GENERIC_MAG_DATA_TLM", 1, GENERIC_MAG_RESPONSE_TIMEOUT)
    sleep(GENERIC_MAG_CMD_SLEEP)
end

def generic_mag_cmd(*command)
    count = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_generic_mag_hk()
    current = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_generic_mag_hk()
        current = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_generic_mag_hk()
            current = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT")
        end
    end
    check("GENERIC_MAG GENERIC_MAG_HK_TLM CMD_COUNT >= #{count}")
end

def enable_generic_mag()
    # Send command
    generic_mag_cmd("GENERIC_MAG GENERIC_MAG_ENABLE_CC")
    # Confirm
    check("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_generic_mag()
    # Send command
    generic_mag_cmd("GENERIC_MAG GENERIC_MAG_DISABLE_CC")
    # Confirm
    check("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_generic_mag()
    get_generic_mag_hk()
    state = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_generic_mag()
    end
end

def confirm_generic_mag_data()
    dev_cmd_cnt = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_ERR_COUNT")
    
    
    # Note these checks assume default simulator configuration
    truth_42_bvb0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA BVB_X_NT")
    get_generic_mag_data()
    check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_X", truth_42_bvb0, GENERIC_MAG_DEVICE_NT_DIFFERENCE)

    truth_42_bvb1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA BVB_Y_NT")
    get_generic_mag_data()
    check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_Y", truth_42_bvb1, GENERIC_MAG_DEVICE_NT_DIFFERENCE)

    truth_42_bvb2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA BVB_Z_NT")
    get_generic_mag_data()
    check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_Z", truth_42_bvb2, GENERIC_MAG_DEVICE_NT_DIFFERENCE)

    get_generic_mag_hk()
    check("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_generic_mag_data_loop()
    GENERIC_MAG_DEVICE_LOOP_COUNT.times do |n|
        confirm_generic_mag_data()
    end
end

#
# Simulator Functions
#
def generic_mag_prepare_ast()
    # Get to known state
    safe_generic_mag()

    # Enable
    enable_generic_mag()

    # Confirm data
    confirm_generic_mag_data_loop()
end

def generic_mag_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE GENERIC_MAG_SIM_ENABLE")
end

def generic_mag_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE GENERIC_MAG_SIM_DISABLE")
end

def generic_mag_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE GENERIC_MAG_SIM_SET_STATUS with STATUS #{status}")
end
