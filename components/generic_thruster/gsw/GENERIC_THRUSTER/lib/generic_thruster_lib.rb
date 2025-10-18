# Library for GENERIC_THRUSTER Target
require 'cosmos'
require 'cosmos/script'

# 
# Definitions
# 
GENERIC_THRUSTER_CMD_SLEEP = 0.25
GENERIC_THRUSTER_RESPONSE_TIMEOUT = 5
GENERIC_THRUSTER_TEST_LOOP_COUNT = 1
GENERIC_THRUSTER_DEVICE_LOOP_COUNT = 1
GENERIC_THRUSTER_DEVICE_DELAY = 3
GENERIC_THRUSTER_DIFF = 0.5

#
# Functions
#
def get_generic_thruster_hk()
    cmd("GENERIC_THRUSTER GENERIC_THRUSTER_REQ_HK")
    wait_check_packet("GENERIC_THRUSTER", "GENERIC_THRUSTER_HK_TLM", 1, GENERIC_THRUSTER_RESPONSE_TIMEOUT)
    sleep(GENERIC_THRUSTER_CMD_SLEEP)
end

def generic_thruster_cmd(*command)
    count = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_generic_thruster_hk()
    current = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT");
    if (current != count)
        # Try again
        cmd(*command)
        get_generic_thruster_hk()
        cmd(*command)
        get_generic_thruster_hk()
        current = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT");
        if (current != count)
            # Third time's the charm?
            cmd(*command)
            get_generic_thruster_hk()
            current = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT");
        end
    end
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM CMD_COUNT >= #{count}")
end

def enable_generic_thruster()
    # Send command
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_ENABLE_CC")
    # Confirm
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM DEVICE_ENABLED == 'ENABLED'")    
end

def disable_generic_thruster()
    # Send command
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_DISABLE_CC")
    # Confirm
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM DEVICE_ENABLED == 'DISABLED'")    
end

def safe_generic_thruster()
    get_generic_thruster_hk()
    state = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 0, PERCENTAGE 0")
        generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 1, PERCENTAGE 0")
        generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 2, PERCENTAGE 0")
        generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 3, PERCENTAGE 0")
        disable_generic_thruster()
    end
end

def confirm_generic_thruster_data()
    dev_cmd_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM DEVICE_ERR_COUNT")

    get_generic_thruster_hk()
    # Note these checks assume default simulator configuration

    truth_42_GYRO_X_initial = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_0").abs()
    truth_42_GYRO_Y_initial = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_1").abs()
    truth_42_GYRO_Z_initial = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_2").abs()

    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 0, PERCENTAGE 10")
    sleep GENERIC_THRUSTER_DEVICE_DELAY
    truth_42_GYRO_X_current = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_0").abs() + GENERIC_THRUSTER_DIFF
    truth_42_GYRO_Y_current = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_1").abs() + GENERIC_THRUSTER_DIFF
    truth_42_GYRO_Z_current = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA WN_2").abs() + GENERIC_THRUSTER_DIFF
    generic_thruster_cmd("GENERIC_THRUSTER GENERIC_THRUSTER_PERCENTAGE_CC with THRUSTER_NUMBER 0, PERCENTAGE 0")

    if (truth_42_GYRO_X_current > truth_42_GYRO_X_initial) || (truth_42_GYRO_Y_current > truth_42_GYRO_Y_initial) ||(truth_42_GYRO_Z_current > truth_42_GYRO_Z_initial)  

    else
        raise "No difference in axis measurements: Test Failed!"
    end

    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_THRUSTER GENERIC_THRUSTER_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_generic_thruster_data_loop()
    GENERIC_THRUSTER_DEVICE_LOOP_COUNT.times do 1
        confirm_generic_thruster_data()
    end
end

#
# Simulator Functions
#
def generic_thruster_prepare_ast()
    # Get to known state
    safe_generic_thruster()

    # Enable
    enable_generic_thruster()

    # Confirm data
    confirm_generic_thruster_data_loop()
end

def generic_thruster_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_SIM_ENABLE")
end

def generic_thruster_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_SIM_DISABLE")
end
