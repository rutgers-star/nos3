# Library for GENERIC_CSS Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
GENERIC_CSS_CMD_SLEEP = 0.25
GENERIC_CSS_RESPONSE_TIMEOUT = 5
GENERIC_CSS_TEST_LOOP_COUNT = 1
GENERIC_CSS_DEVICE_LOOP_COUNT = 5
GENERIC_CSS_DEVICE_TRUTH_MARGIN = 100

#
# Functions
#
def get_generic_css_hk()
    cmd("GENERIC_CSS GENERIC_CSS_REQ_HK")
    wait_check_packet("GENERIC_CSS", "GENERIC_CSS_HK_TLM", 1, GENERIC_CSS_RESPONSE_TIMEOUT)
    sleep(GENERIC_CSS_CMD_SLEEP)
end

def get_generic_css_data()
    cmd("GENERIC_CSS GENERIC_CSS_REQ_DATA")
    wait_check_packet("GENERIC_CSS", "GENERIC_CSS_DATA_TLM", 1, GENERIC_CSS_RESPONSE_TIMEOUT)
    sleep(GENERIC_CSS_CMD_SLEEP)
end

def generic_css_cmd(*command)
    count = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_generic_css_hk()
    current = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_generic_css_hk()
        current = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_generic_css_hk()
            current = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT")
        end
    end
    check("GENERIC_CSS GENERIC_CSS_HK_TLM CMD_COUNT >= #{count}")
end

def enable_generic_css()
    # Send command
    generic_css_cmd("GENERIC_CSS GENERIC_CSS_ENABLE_CC")
    # Confirm
    check("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_generic_css()
    # Send command
    generic_css_cmd("GENERIC_CSS GENERIC_CSS_DISABLE_CC")
    # Confirm
    check("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_generic_css()
    get_generic_css_hk()
    state = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_generic_css()
    end
end

def confirm_generic_css_data()
    dev_cmd_cnt = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_ERR_COUNT")
    
    # Note these checks assume truth data from 42 is available, and that the spacecraft is not rapidly tumbling
    # The CSS orientations were taken from the ./cfg/InOut/SC_NOS3.txt
    in_sun = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA IN_SUN")
    if(in_sun > 0)
        # CSS 0,  1, 0, 0
        svb_0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_0")
        get_generic_css_data()
        if(svb_0 > 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_0", svb_0 * 1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_0", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        end

        # CSS 1, -1, 0, 0
        svb_0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_0")
        get_generic_css_data()
        if(svb_0 < 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_1", svb_0 * -1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_1", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        end

        # CSS 2,  0, 1, 0
        svb_1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_1")
        get_generic_css_data()
        if(svb_1 > 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_2", svb_1 * 1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_2", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        end

        # CSS 3,  0,-1, 0
        svb_1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_1")
        get_generic_css_data()
        if(svb_1 < 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_3", svb_1 * -1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_3", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        end

        # CSS 4,  0, 0, 1
        svb_2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_2")
        get_generic_css_data()
        if(svb_2 > 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_4", svb_2 * 1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_4", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        end

        # CSS 5,  0, 0,-1
        svb_2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_2")
        get_generic_css_data()
        if(svb_2 < 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_5", svb_2 * -1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_5", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
        end
    end

    # Confirm no errors occurred
    get_generic_css_hk()
    check("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_generic_css_data_loop()
    GENERIC_CSS_DEVICE_LOOP_COUNT.times do |n|
        confirm_generic_css_data()
    end
end

#
# Simulator Functions
#
def generic_css_prepare_ast()
    # Get to known state
    safe_generic_css()

    # Enable
    enable_generic_css()

    # Confirm data
    confirm_generic_css_data_loop()
end

def generic_css_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE GENERIC_CSS_SIM_ENABLE")
end

def generic_css_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE GENERIC_CSS_SIM_DISABLE")
end

def generic_css_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE GENERIC_CSS_SIM_SET_STATUS with STATUS #{status}")
end
