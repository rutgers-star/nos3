# Library for TMP100 Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
TMP100_CMD_SLEEP = 0.25
TMP100_RESPONSE_TIMEOUT = 5
TMP100_TEST_LOOP_COUNT = 1
TMP100_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_tmp100_hk()
    cmd("TMP100 TMP100_REQ_HK")
    wait_check_packet("TMP100", "TMP100_HK_TLM", 1, TMP100_RESPONSE_TIMEOUT)
    sleep(TMP100_CMD_SLEEP)
end

def get_tmp100_data()
    cmd("TMP100 TMP100_REQ_DATA")
    wait_check_packet("TMP100", "TMP100_DATA_TLM", 1, TMP100_RESPONSE_TIMEOUT)
    sleep(TMP100_CMD_SLEEP)
end

def tmp100_cmd(*command)
    count = tlm("TMP100 TMP100_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_tmp100_hk()
    current = tlm("TMP100 TMP100_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_tmp100_hk()
        current = tlm("TMP100 TMP100_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_tmp100_hk()
            current = tlm("TMP100 TMP100_HK_TLM CMD_COUNT")
        end
    end
    check("TMP100 TMP100_HK_TLM CMD_COUNT >= #{count}")
end

def enable_tmp100()
    # Send command
    tmp100_cmd("TMP100 TMP100_ENABLE_CC")
    # Confirm
    check("TMP100 TMP100_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_tmp100()
    # Send command
    tmp100_cmd("TMP100 TMP100_DISABLE_CC")
    # Confirm
    check("TMP100 TMP100_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_tmp100()
    get_tmp100_hk()
    state = tlm("TMP100 TMP100_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_tmp100()
    end
end

def confirm_tmp100_temperature()
    dev_cmd_cnt = tlm("TMP100 TMP100_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("TMP100 TMP100_HK_TLM DEVICE_ERR_COUNT")

    get_tmp100_hk()
    # Check temperature is in expected range (20-30°C from simulator)
    temp_c = tlm("TMP100 TMP100_HK_TLM TEMPERATURE_C")
    check("TMP100 TMP100_HK_TLM TEMPERATURE_C >= 15.0")  # Allow margin
    check("TMP100 TMP100_HK_TLM TEMPERATURE_C <= 35.0")  # Allow margin
    check("TMP100 TMP100_HK_TLM RAW_TEMPERATURE != 0")   # Should have valid data

    puts "TMP100 Temperature: #{temp_c.round(2)} °C"

    check("TMP100 TMP100_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_tmp100_data()
    dev_cmd_cnt = tlm("TMP100 TMP100_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("TMP100 TMP100_HK_TLM DEVICE_ERR_COUNT")

    get_tmp100_data()
    # Check temperature data packet
    temp_c = tlm("TMP100 TMP100_DATA_TLM TEMPERATURE_CELSIUS")
    check("TMP100 TMP100_DATA_TLM TEMPERATURE_CELSIUS >= 15.0")
    check("TMP100 TMP100_DATA_TLM TEMPERATURE_CELSIUS <= 35.0")
    check("TMP100 TMP100_DATA_TLM RAW_TEMPERATURE != 0")

    puts "TMP100 Temperature Data: #{temp_c.round(2)} °C"

    get_tmp100_hk()
    check("TMP100 TMP100_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("TMP100 TMP100_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_tmp100_data_loop()
    TMP100_DEVICE_LOOP_COUNT.times do |n|
        confirm_tmp100_temperature()
        confirm_tmp100_data()
    end
end

#
# Simulator Functions
#
def tmp100_prepare_ast()
    # Get to known state
    safe_tmp100()

    # Enable
    enable_tmp100()

    # Confirm data
    confirm_tmp100_data_loop()
end

def tmp100_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE TMP100_SIM_ENABLE")
end

def tmp100_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE TMP100_SIM_DISABLE")
end

def tmp100_sim_set_temperature(temp_celsius)
    cmd("SIM_CMDBUS_BRIDGE TMP100_SIM_CMD with CMD_STRING \"TEMPERATURE=#{temp_celsius}\"")
    puts "TMP100 Simulator: Set temperature to #{temp_celsius} °C"
end
