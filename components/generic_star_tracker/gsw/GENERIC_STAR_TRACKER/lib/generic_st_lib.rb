# Library for GENERIC_STAR_TRACKER Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
GENERIC_STAR_TRACKER_CMD_SLEEP = 0.25
GENERIC_STAR_TRACKER_RESPONSE_TIMEOUT = 5
GENERIC_STAR_TRACKER_TEST_LOOP_COUNT = 1
GENERIC_STAR_TRACKER_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_generic_star_tracker_hk()
    cmd("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_REQ_HK")
    wait_check_packet("GENERIC_STAR_TRACKER", "GENERIC_STAR_TRACKER_HK_TLM", 1, GENERIC_STAR_TRACKER_RESPONSE_TIMEOUT)
    sleep(GENERIC_STAR_TRACKER_CMD_SLEEP)
end

def get_generic_star_tracker_data()
    cmd("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_REQ_DATA")
    wait_check_packet("GENERIC_STAR_TRACKER", "GENERIC_STAR_TRACKER_DATA_TLM", 1, GENERIC_STAR_TRACKER_RESPONSE_TIMEOUT)
    sleep(GENERIC_STAR_TRACKER_CMD_SLEEP)
end

def generic_star_tracker_cmd(*command)
    count = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_generic_star_tracker_hk()
    current = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_generic_star_tracker_hk()
        current = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_generic_star_tracker_hk()
            current = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM CMD_COUNT")
        end
    end
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM CMD_COUNT >= #{count}")
end

def enable_generic_star_tracker()
    # Send command
    generic_star_tracker_cmd("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_ENABLE_CC")
    # Confirm
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_generic_star_tracker()
    # Send command
    generic_star_tracker_cmd("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DISABLE_CC")
    # Confirm
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_generic_star_tracker()
    get_generic_star_tracker_hk()
    state = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_generic_star_tracker()
    end
end

def confirm_generic_star_tracker_data()
    dev_cmd_cnt = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ERR_COUNT")
    
    get_generic_star_tracker_data()
    # Note these checks assume default simulator configuration
    diff = 1 
    #Todo check values with lower margin
    #Margin is large, testing that Q values not equal to 0

    #Checking Q0
    st_Q0 = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q0")
    sim42_Q0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_0")
    puts "sim42_q0 is #{sim42_Q0}"
    puts "st_q0 is #{st_Q0}"

    wait_check_tolerance("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q0", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_0"), diff, 30)

    #Checking Q1
    st_Q1 = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q1")
    sim42_Q1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_1")
    puts "sim42_q1 is #{sim42_Q1}"
    puts "st_q1 is #{st_Q1}"

    wait_check_tolerance("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q1", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_1"), diff, 30)

    #Checking Q2
    st_Q2 = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q2")
    sim42_Q2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_2")
    puts "sim42_q2 is #{sim42_Q2}"
    puts "st_q2 is #{st_Q2}"

    wait_check_tolerance("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q2", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_2"), diff, 30)


    #Checking Q3
    st_Q3 = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q3")
    sim42_Q3 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_3")
    puts "sim42_q3 is #{sim42_Q3}"
    puts "st_q3 is #{st_Q3}"

    wait_check_tolerance("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q3", tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_3"), diff, 30)



    get_generic_star_tracker_hk()
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_generic_star_tracker_data_loop()
    GENERIC_STAR_TRACKER_DEVICE_LOOP_COUNT.times do |n|
        confirm_generic_star_tracker_data()
    end
end

#
# Simulator Functions
#
def generic_star_tracker_prepare_ast()
    # Get to known state
    safe_generic_star_tracker()

    # Enable
    enable_generic_star_tracker()

    # Confirm data
    confirm_generic_star_tracker_data_loop()
end

def generic_star_tracker_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE GENERIC_STAR_TRACKER_SIM_ENABLE")
end

def generic_star_tracker_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE GENERIC_STAR_TRACKER_SIM_DISABLE")
end

def generic_star_tracker_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE GENERIC_STAR_TRACKER_SIM_SET_STATUS with STATUS #{status}")
end
