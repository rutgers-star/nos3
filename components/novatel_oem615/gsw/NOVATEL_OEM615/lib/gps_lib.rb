# Library for NOVATEL_OEM615 Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
NOVATEL_OEM615_CMD_SLEEP = 0.33
NOVATEL_OEM615_RESPONSE_TIMEOUT = 5
NOVATEL_OEM615_TEST_LOOP_COUNT = 1
NOVATEL_OEM615_DEVICE_LOOP_COUNT = 5
NOVATEL_OEM615_POSITION_DIFF = 25000
NOVATEL_OEM615_VELOCITY_DIFF = 100

#
# Functions
#
def get_gps_hk()
    cmd("NOVATEL_OEM615 NOVATEL_OEM615_REQ_HK")
    wait_check_packet("NOVATEL_OEM615", "NOVATEL_OEM615_HK_TLM", 1, NOVATEL_OEM615_RESPONSE_TIMEOUT)
    sleep(NOVATEL_OEM615_CMD_SLEEP)
end

def gps_cmd(*command)
    count = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_gps_hk()
    current = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_gps_hk()
        current = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_gps_hk()
            current = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT")
        end
    end
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM CMD_COUNT >= #{count}")
end

def enable_gps()
    # Send command
    gps_cmd("NOVATEL_OEM615 NOVATEL_OEM615_ENABLE_CC")
    # Confirm
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_gps()
    # Send command
    gps_cmd("NOVATEL_OEM615 NOVATEL_OEM615_DISABLE_CC")
    # Confirm
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_gps()
    get_gps_hk()
    state = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_gps()
    end
end

def confirm_gps_data()
    dev_cmd_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_ERR_COUNT")
    
    wait_check_tolerance("NOVATEL_OEM615 NOVATEL_OEM615_DATA_TLM ECEF_X", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA POSITION_W_0"), NOVATEL_OEM615_POSITION_DIFF, NOVATEL_OEM615_RESPONSE_TIMEOUT)
    wait_check_tolerance("NOVATEL_OEM615 NOVATEL_OEM615_DATA_TLM ECEF_Y", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA POSITION_W_1"), NOVATEL_OEM615_POSITION_DIFF, NOVATEL_OEM615_RESPONSE_TIMEOUT)
    wait_check_tolerance("NOVATEL_OEM615 NOVATEL_OEM615_DATA_TLM ECEF_Z", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA POSITION_W_2"), NOVATEL_OEM615_POSITION_DIFF, NOVATEL_OEM615_RESPONSE_TIMEOUT)

    wait_check_tolerance("NOVATEL_OEM615 NOVATEL_OEM615_DATA_TLM VEL_X", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA VELOCITY_W_0"), NOVATEL_OEM615_VELOCITY_DIFF, NOVATEL_OEM615_RESPONSE_TIMEOUT)
    wait_check_tolerance("NOVATEL_OEM615 NOVATEL_OEM615_DATA_TLM VEL_Y", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA VELOCITY_W_1"), NOVATEL_OEM615_VELOCITY_DIFF, NOVATEL_OEM615_RESPONSE_TIMEOUT)
    wait_check_tolerance("NOVATEL_OEM615 NOVATEL_OEM615_DATA_TLM VEL_Z", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA VELOCITY_W_2"), NOVATEL_OEM615_VELOCITY_DIFF, NOVATEL_OEM615_RESPONSE_TIMEOUT)

    get_gps_hk()
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_gps_data_loop()
    NOVATEL_OEM615_DEVICE_LOOP_COUNT.times do |n|
        confirm_gps_data()
    end
end

#
# Simulator Functions
#
def gps_prepare_ast()
    # Get to known state
    safe_gps()

    # Enable
    enable_gps()

    # Confirm data
    confirm_gps_data_loop()
end

def gps_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE NOVATEL_OEM615_SIM_ENABLE")
end

def gps_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE NOVATEL_OEM615_SIM_DISABLE")
end

def gps_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE NOVATEL_OEM615_SIM_SET_STATUS with STATUS #{status}")
end
