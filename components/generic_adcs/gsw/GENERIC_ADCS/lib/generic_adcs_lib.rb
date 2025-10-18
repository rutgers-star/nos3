# Library for GENERIC_ADCS Target
require 'cosmos'
require 'cosmos/script'
require 'generic_css_lib.rb'
require 'generic_fss_lib.rb'
require 'generic_imu_lib.rb'
require 'generic_mag_lib.rb'
require 'generic_reaction_wheel_lib.rb'
require 'generic_st_lib.rb'
require 'gps_lib.rb'

#
# Definitions
#
GENERIC_ADCS_CMD_SLEEP = 0.25
GENERIC_ADCS_RESPONSE_TIMEOUT = 5
GENERIC_ADCS_MODE_CHECK_TIMEOUT = 60;
GENERIC_ADCS_TEST_LOOP_COUNT = 1
GENERIC_ADCS_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_adcs_hk()
    cmd("GENERIC_ADCS GENERIC_ADCS_REQ_HK")
    wait_check_packet("GENERIC_ADCS", "GENERIC_ADCS_HK_TLM", 1, GENERIC_ADCS_RESPONSE_TIMEOUT)
    sleep(GENERIC_ADCS_CMD_SLEEP)
end

def get_adcs_data()
    cmd("GENERIC_ADCS GENERIC_ADCS_SEND_DI_CC")
    wait_check_packet("GENERIC_ADCS", "GENERIC_ADCS_DI", 1, GENERIC_ADCS_RESPONSE_TIMEOUT)
    cmd("GENERIC_ADCS GENERIC_ADCS_SEND_GNC_CC")
    wait_check_packet("GENERIC_ADCS", "GENERIC_ADCS_GNC", 1, GENERIC_ADCS_RESPONSE_TIMEOUT)
    cmd("GENERIC_ADCS GENERIC_ADCS_SEND_AD_CC")
    wait_check_packet("GENERIC_ADCS", "GENERIC_ADCS_AD", 1, GENERIC_ADCS_RESPONSE_TIMEOUT)
    cmd("GENERIC_ADCS GENERIC_ADCS_SEND_AC_CC")
    wait_check_packet("GENERIC_ADCS", "GENERIC_ADCS_AC", 1, GENERIC_ADCS_RESPONSE_TIMEOUT)
    cmd("GENERIC_ADCS GENERIC_ADCS_SEND_DO_CC")
    wait_check_packet("GENERIC_ADCS", "GENERIC_ADCS_DO", 1, GENERIC_ADCS_RESPONSE_TIMEOUT)
    sleep(GENERIC_ADCS_CMD_SLEEP)
end

def adcs_cmd(*command)
    count = tlm("GENERIC_ADCS GENERIC_ADCS_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_adcs_hk()
    current = tlm("GENERIC_ADCS GENERIC_ADCS_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_adcs_hk()
        current = tlm("GENERIC_ADCS GENERIC_ADCS_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_adcs_hk()
            current = tlm("GENERIC_ADCS GENERIC_ADCS_HK_TLM CMD_COUNT")
        end
    end
    check("GENERIC_ADCS GENERIC_ADCS_HK_TLM CMD_COUNT >= #{count}")
end

def adcs_sunsafe()

    cmd("GENERIC_ADCS GENERIC_ADCS_SET_MODE_CC with GNC_MODE SUNSAFE_MODE")

end

def adcs_bdot()

    cmd("GENERIC_ADCS GENERIC_ADCS_SET_MODE_CC with GNC_MODE BDOT_MODE")

end

def adcs_inertial()

    cmd("GENERIC_ADCS GENERIC_ADCS_SET_MODE_CC with GNC_MODE INERTIAL_MODE")

end

def adcs_passive()

    cmd("GENERIC_ADCS GENERIC_ADCS_SET_MODE_CC with GNC_MODE PASSIVE")

end

def adcs_set_q()

    cmd("GENERIC_ADCS GENERIC_ADCS_INERTIAL_QUATERNION_CC with GNC_INER_QUAT1 0.0, GNC_INER_QUAT2 0.0, GNC_INER_QUAT3 0.0, GNC_INER_QUAT4 1.0")

end

def safe_adcs()

    cmd("GENERIC_FSS GENERIC_FSS_REQ_HK")
    fss_enabled = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ENABLED")
    if (fss_enabled != "ENABLED")
        cmd("GENERIC_FSS GENERIC_FSS_ENABLE_CC")
    end

    cmd("GENERIC_CSS GENERIC_CSS_REQ_HK")
    css_enabled = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_ENABLED")
    if (css_enabled != "ENABLED")
        cmd("GENERIC_CSS GENERIC_CSS_ENABLE_CC")
    end

    cmd("GENERIC_IMU GENERIC_IMU_REQ_HK")
    imu_enabled = tlm("GENERIC_IMU GENERIC_IMU_HK_TLM DEVICE_ENABLED")
    if (imu_enabled != "ENABLED")
        cmd("GENERIC_IMU GENERIC_IMU_ENABLE_CC")
    end

    cmd("GENERIC_MAG GENERIC_MAG_REQ_HK")
    mag_enabled = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_ENABLED")
    if (mag_enabled != "ENABLED")
        cmd("GENERIC_MAG GENERIC_MAG_ENABLE_CC")
    end

    sw1_state = tlm("GENERIC_EPS GENERIC_EPS_HK_TLM SWITCH_1_STATE")
    if(sw1_state == "OFF")
        eps_cmd("GENERIC_EPS GENERIC_EPS_SWITCH_CC with SWITCH_NUMBER SWITCH_1, STATE ON")
    end

    cmd("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_REQ_HK")
    st_enabled = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ENABLED")
    if (st_enabled != "ENABLED")
        cmd("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_ENABLE_CC")
    end

    cmd("NOVATEL_OEM615 NOVATEL_OEM615_REQ_HK")
    gps_enabled = tlm("NOVATEL_OEM615 NOVATEL_OEM615_HK_TLM DEVICE_ENABLED")
    if (gps_enabled != "ENABLED")
        cmd("NOVATEL_OEM615 NOVATEL_OEM615_ENABLE_CC")
    end

    cmd("GENERIC_TORQUER GENERIC_TORQUER_REQ_HK_CC")
    torquer_enabled = tlm("GENERIC_TORQUER GENERIC_TORQUER_HK_TLM_T DEVICE_ENABLED")
    if (torquer_enabled != "ENABLED")
        cmd("GENERIC_TORQUER GENERIC_TORQUER_ENABLE_CC")
    end

    get_adcs_data()
    mode = tlm("GENERIC_ADCS GENERIC_ADCS_GNC MODE")
    if (mode != "SUNSAFE")
        adcs_sunsafe()
    end
end

def confirm_adcs_data()
    
    adcs_sunsafe()

    diff = 0.05

    get_adcs_data()
    adcs_sun_valid = tlm("GENERIC_ADCS GENERIC_ADCS_GNC SUN_VALID")
    if adcs_sun_valid == 0
        wait_check_tolerance("GENERIC_ADCS GENERIC_ADCS_GNC SVB_X", 1.0, diff, GENERIC_ADCS_MODE_CHECK_TIMEOUT)
        wait_check_tolerance("GENERIC_ADCS GENERIC_ADCS_GNC SVB_Y", 0.0, diff, GENERIC_ADCS_MODE_CHECK_TIMEOUT)
        wait_check_tolerance("GENERIC_ADCS GENERIC_ADCS_GNC SVB_Z", 0.0, diff, GENERIC_ADCS_MODE_CHECK_TIMEOUT)

        diff = 50
        wait_check_tolerance("GENERIC_ADCS GENERIC_ADCS_GNC CSS_0", 1000, diff, GENERIC_ADCS_MODE_CHECK_TIMEOUT)
    end

    cmd("GENERIC_IMU GENERIC_IMU_REQ_DATA")

    x_ang_rate_before = tlm("GENERIC_IMU GENERIC_IMU_DATA_TLM X_ANGULAR_RATE").abs
    y_ang_rate_before = tlm("GENERIC_IMU GENERIC_IMU_DATA_TLM Y_ANGULAR_RATE").abs
    z_ang_rate_before = tlm("GENERIC_IMU GENERIC_IMU_DATA_TLM Z_ANGULAR_RATE").abs

    adcs_bdot()

    diff = 0.05
    
    wait_check("GENERIC_IMU GENERIC_IMU_DATA_TLM X_ANGULAR_RATE < #{x_ang_rate_before}", GENERIC_ADCS_MODE_CHECK_TIMEOUT)
    wait_check("GENERIC_IMU GENERIC_IMU_DATA_TLM Y_ANGULAR_RATE < #{y_ang_rate_before}", GENERIC_ADCS_MODE_CHECK_TIMEOUT)
    wait_check("GENERIC_IMU GENERIC_IMU_DATA_TLM Z_ANGULAR_RATE < #{z_ang_rate_before}", GENERIC_ADCS_MODE_CHECK_TIMEOUT)

    adcs_inertial()
    adcs_set_q();

    sleep(GENERIC_ADCS_MODE_CHECK_TIMEOUT)

    qbn0 = tlm("GENERIC_ADCS GENERIC_ADCS_GNC QBN_0").abs
    qbn1 = tlm("GENERIC_ADCS GENERIC_ADCS_GNC QBN_1").abs
    qbn2 = tlm("GENERIC_ADCS GENERIC_ADCS_GNC QBN_2").abs
    qbn3 = tlm("GENERIC_ADCS GENERIC_ADCS_GNC QBN_3").abs

    wait_check_expression("#{qbn0} < 0.1", GENERIC_ADCS_MODE_CHECK_TIMEOUT)
    wait_check_expression("#{qbn1} < 0.1", GENERIC_ADCS_MODE_CHECK_TIMEOUT)
    wait_check_expression("#{qbn2} < 0.1", GENERIC_ADCS_MODE_CHECK_TIMEOUT)
    wait_check_expression("#{qbn3} > 0.9", GENERIC_ADCS_MODE_CHECK_TIMEOUT)

    get_adcs_hk()
end

def adcs_confirm_css_data()

    dev_cmd_cnt = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_CSS GENERIC_CSS_HK_TLM DEVICE_ERR_COUNT")

    diff = 0.05
    
    # Note these checks assume truth data from 42 is available, and that the spacecraft is not rapidly tumbling
    # The CSS orientations were taken from the ./cfg/InOut/SC_NOS3.txt
    in_sun = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA IN_SUN")
    if(in_sun > 0)
        # CSS 0,  1, 0, 0
        svb_0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_0")
        get_generic_css_data()
        get_adcs_data()
        if(svb_0 > 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_0", svb_0 * 1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css0_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_0")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON0", css0_val, diff)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_0", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css0_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_0")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON0", css0_val, diff)
        end

        # CSS 1, -1, 0, 0
        svb_0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_0")
        get_generic_css_data()
        get_adcs_data()
        if(svb_0 < 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_1", svb_0 * -1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css1_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_1")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON1", css1_val, diff)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_1", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css1_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_1")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON1", css1_val, diff)
        end

        # CSS 2,  0, 1, 0
        svb_1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_1")
        get_generic_css_data()
        get_adcs_data()
        if(svb_1 > 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_2", svb_1 * 1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css2_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_2")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON2", css2_val, diff)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_2", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css2_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_2")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON2", css2_val, diff)
        end

        # CSS 3,  0,-1, 0
        svb_1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_1")
        get_generic_css_data()
        get_adcs_data()
        if(svb_1 < 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_3", svb_1 * -1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css3_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_3")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON3", css3_val, diff)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_3", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css3_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_3")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON3", css3_val, diff)
        end

        # CSS 4,  0, 0, 1
        svb_2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_2")
        get_generic_css_data()
        get_adcs_data()
        if(svb_2 > 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_4", svb_2 * 1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css4_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_4")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON4", css4_val, diff)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_4", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css4_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_4")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON4", css4_val, diff)
        end

        # CSS 5,  0, 0,-1
        svb_2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_2")
        get_generic_css_data()
        get_adcs_data()
        if(svb_2 < 0)
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_5", svb_2 * -1000, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css5_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_5")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON5", css5_val, diff)
        else
            check_tolerance("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_5", 0, GENERIC_CSS_DEVICE_TRUTH_MARGIN)
            css5_val = tlm("GENERIC_CSS GENERIC_CSS_DATA_TLM RAW_CSS_5")/1000.0
            check_tolerance("GENERIC_ADCS GENERIC_ADCS_DI PERCENTON5", css5_val, diff)
        end
    end
end

def adcs_confirm_fss_data()
    
    dev_cmd_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
    
    get_fss_data()
    get_adcs_data()
    truth_svb0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_0")
    truth_svb1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_1")
    truth_svb2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_2")

    adcs_svb0 = tlm("GENERIC_ADCS GENERIC_ADCS_DI FSS_SVB_X")
    adcs_svb1 = tlm("GENERIC_ADCS GENERIC_ADCS_DI FSS_SVB_Y")
    adcs_svb2 = tlm("GENERIC_ADCS GENERIC_ADCS_DI FSS_SVB_Z")

    fss_error = tlm("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_ERROR_CODE")

    wait_check("GENERIC_ADCS GENERIC_ADCS_DI FSS_VALID == 1", GENERIC_ADCS_MODE_CHECK_TIMEOUT)

    truth_42_alpha = -Math.atan2(truth_svb2, truth_svb0)
    truth_42_beta = Math.atan2(truth_svb1, truth_svb0)

    adcs_alpha = -Math.atan2(adcs_svb2, adcs_svb0)
    adcs_beta = Math.atan2(adcs_svb1, adcs_svb0)

    diff = 0.03

    if fss_error == 0
        wait_check_tolerance("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_ALPHA", truth_42_alpha, diff, GENERIC_FSS_RESPONSE_TIMEOUT)

        wait_check_tolerance("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_ALPHA", adcs_alpha, diff, GENERIC_FSS_RESPONSE_TIMEOUT)

        wait_check_tolerance("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_BETA", truth_42_beta, diff, GENERIC_FSS_RESPONSE_TIMEOUT)

        wait_check_tolerance("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_BETA", adcs_beta, diff, GENERIC_FSS_RESPONSE_TIMEOUT)
    end

    get_fss_hk()
    check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
    
end

def adcs_confirm_imu_data()
    
    dev_cmd_cnt = tlm("GENERIC_IMU GENERIC_IMU_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_IMU GENERIC_IMU_HK_TLM DEVICE_ERR_COUNT")
    
    get_generic_imu_data()
    get_adcs_data()
    # Note these checks assume default simulator configuration

    # X Axis Angular
    truth_42_GYRO_X = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA GYRO_B_X")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM X_ANGULAR_RATE", truth_42_GYRO_X, GENERIC_IMU_DEVICE_ANGULAR_DIFF)
    adcs_GYRO_X = tlm("GENERIC_ADCS GENERIC_ADCS_DI IMU_WBN_X")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM X_ANGULAR_RATE", adcs_GYRO_X, GENERIC_IMU_DEVICE_ANGULAR_DIFF)

    # Y Axis Angular
    truth_42_GYRO_Y = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA GYRO_B_Y")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM Y_ANGULAR_RATE", truth_42_GYRO_Y, GENERIC_IMU_DEVICE_ANGULAR_DIFF)
    adcs_GYRO_Y = tlm("GENERIC_ADCS GENERIC_ADCS_DI IMU_WBN_Y")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM Y_ANGULAR_RATE", adcs_GYRO_Y, GENERIC_IMU_DEVICE_ANGULAR_DIFF)

    # Z Axis Angular
    truth_42_GYRO_Z = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA GYRO_B_Z")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM Z_ANGULAR_RATE", truth_42_GYRO_Z, GENERIC_IMU_DEVICE_ANGULAR_DIFF)
    adcs_GYRO_Z = tlm("GENERIC_ADCS GENERIC_ADCS_DI IMU_WBN_Z")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM Z_ANGULAR_RATE", adcs_GYRO_Z, GENERIC_IMU_DEVICE_ANGULAR_DIFF)

    # X Axis Linear
    truth_42_ACC_X = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA ACC_B_X")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM X_LINEAR_ACCELERATION", truth_42_ACC_X, GENERIC_IMU_DEVICE_LINEAR_DIFF)
    adcs_ACC_X = tlm("GENERIC_ADCS GENERIC_ADCS_DI IMU_ACC_X")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM X_LINEAR_ACCELERATION", adcs_ACC_X, GENERIC_IMU_DEVICE_LINEAR_DIFF)

    # Y Axis Linear
    truth_42_ACC_Y = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA ACC_B_Y")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM Y_LINEAR_ACCELERATION", truth_42_ACC_Y, GENERIC_IMU_DEVICE_LINEAR_DIFF)
    adcs_ACC_Y = tlm("GENERIC_ADCS GENERIC_ADCS_DI IMU_ACC_Y")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM Y_LINEAR_ACCELERATION", adcs_ACC_Y, GENERIC_IMU_DEVICE_LINEAR_DIFF)

    # Z Axis Linear
    truth_42_ACC_Z = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA ACC_B_Z")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM Z_LINEAR_ACCELERATION", truth_42_ACC_Z, GENERIC_IMU_DEVICE_LINEAR_DIFF)
    adcs_ACC_Z = tlm("GENERIC_ADCS GENERIC_ADCS_DI IMU_ACC_Z")
    check_tolerance("GENERIC_IMU GENERIC_IMU_DATA_TLM Z_LINEAR_ACCELERATION", adcs_ACC_Z, GENERIC_IMU_DEVICE_LINEAR_DIFF)

    get_generic_imu_hk()
    check("GENERIC_IMU GENERIC_IMU_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_IMU GENERIC_IMU_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
    
end

def adcs_confirm_mag_data()
    
    dev_cmd_cnt = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_ERR_COUNT")
    
    
    # Note these checks assume default simulator configuration
    truth_42_bvb0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA BVB_X_NT")
    get_generic_mag_data()
    get_adcs_data()
    check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_X", truth_42_bvb0, GENERIC_MAG_DEVICE_NT_DIFFERENCE)
    adcs_bvb0 = tlm("GENERIC_ADCS GENERIC_ADCS_DI BVB_X")
    wait_check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_X", adcs_bvb0 * 1000000000, GENERIC_MAG_DEVICE_NT_DIFFERENCE, GENERIC_ADCS_RESPONSE_TIMEOUT)


    truth_42_bvb1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA BVB_Y_NT")
    get_generic_mag_data()
    get_adcs_data()
    check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_Y", truth_42_bvb1, GENERIC_MAG_DEVICE_NT_DIFFERENCE)
    adcs_bvb1 = tlm("GENERIC_ADCS GENERIC_ADCS_DI BVB_Y")
    wait_check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_Y", adcs_bvb1 * 1000000000, GENERIC_MAG_DEVICE_NT_DIFFERENCE, GENERIC_ADCS_RESPONSE_TIMEOUT)

    truth_42_bvb2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA BVB_Z_NT")
    get_generic_mag_data()
    get_adcs_data()
    check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_Z", truth_42_bvb2, GENERIC_MAG_DEVICE_NT_DIFFERENCE)
    adcs_bvb2 = tlm("GENERIC_ADCS GENERIC_ADCS_DI BVB_Z")
    wait_check_tolerance("GENERIC_MAG GENERIC_MAG_DATA_TLM RAW_MAG_Z", adcs_bvb2 * 1000000000, GENERIC_MAG_DEVICE_NT_DIFFERENCE, GENERIC_ADCS_RESPONSE_TIMEOUT)

    get_generic_mag_hk()
    check("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_MAG GENERIC_MAG_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
    
end

def adcs_confirm_rw_data()

    diff = 0.05

    get_adcs_data()
    get_GENERIC_REACTION_WHEEL_data()
    adcs_rw0 = tlm("GENERIC_ADCS GENERIC_ADCS_DO TCMD_X")
    wait_check_tolerance("GENERIC_REACTION_WHEEL GENRW_HK_TLM_T MOMENTUM_NMS_0", adcs_rw0, diff, GENERIC_ADCS_MODE_CHECK_TIMEOUT)

    get_adcs_data()
    get_GENERIC_REACTION_WHEEL_data()
    adcs_rw1 = tlm("GENERIC_ADCS GENERIC_ADCS_DO TCMD_Y")
    wait_check_tolerance("GENERIC_REACTION_WHEEL GENRW_HK_TLM_T MOMENTUM_NMS_1", adcs_rw1, diff, GENERIC_ADCS_MODE_CHECK_TIMEOUT)

    get_adcs_data()
    get_GENERIC_REACTION_WHEEL_data()
    adcs_rw2 = tlm("GENERIC_ADCS GENERIC_ADCS_DO TCMD_Z")
    wait_check_tolerance("GENERIC_REACTION_WHEEL GENRW_HK_TLM_T MOMENTUM_NMS_2", adcs_rw2, diff, GENERIC_ADCS_MODE_CHECK_TIMEOUT)

end

def adcs_confirm_st_data()
    
    dev_cmd_cnt = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ERR_COUNT")
    
    get_generic_star_tracker_data()
    get_adcs_data()
    # Note these checks assume default simulator configuration
    diff = 0.05
    #Todo check values with lower margin
    #Margin is large, testing that Q values not equal to 0

    #Checking Q0
    st_Q0 = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q0").abs
    sim42_Q0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_0").abs
    adcs_Q0 = tlm("GENERIC_ADCS GENERIC_ADCS_DI ST_Q_0").abs
    puts "sim42_q0 is #{sim42_Q0}"
    puts "adcs_q0 is #{adcs_Q0}"
    puts "st_q0 is #{st_Q0}"

    wait_check_expression("#{st_Q0} < #{sim42_Q0 + 0.05} or #{st_Q0} > #{sim42_Q0 - 0.05}", 30)
    wait_check_expression("#{st_Q0} < #{adcs_Q0 + 0.05} or #{st_Q0} > #{adcs_Q0 - 0.05}", 30)

    #Checking Q1
    st_Q1 = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q1").abs
    sim42_Q1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_1").abs
    adcs_Q1 = tlm("GENERIC_ADCS GENERIC_ADCS_DI ST_Q_1").abs
    puts "sim42_q1 is #{sim42_Q1}"
    puts "adcs_q1 is #{adcs_Q1}"
    puts "st_q1 is #{st_Q1}"

    wait_check_expression("#{st_Q1} < #{sim42_Q1 + 0.05} or #{st_Q1} > #{sim42_Q1 - 0.05}", 30)
    wait_check_expression("#{st_Q1} < #{adcs_Q1 + 0.05} or #{st_Q1} > #{adcs_Q1 - 0.05}", 30)

    #Checking Q2
    st_Q2 = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q2").abs
    sim42_Q2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_2").abs
    adcs_Q2 = tlm("GENERIC_ADCS GENERIC_ADCS_DI ST_Q_2").abs
    puts "sim42_q2 is #{sim42_Q2}"
    puts "adcs_q2 is #{adcs_Q2}"
    puts "st_q2 is #{st_Q2}"

    wait_check_expression("#{st_Q2} < #{sim42_Q2 + 0.05} or #{st_Q2} > #{sim42_Q2 - 0.05}", 30)
    wait_check_expression("#{st_Q2} < #{adcs_Q2 + 0.05} or #{st_Q2} > #{adcs_Q2 - 0.05}", 30)


    #Checking Q3
    st_Q3 = tlm("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_DATA_TLM STAR_TRACKER_Q3").abs
    sim42_Q3 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA QN_3").abs
    adcs_Q3 = tlm("GENERIC_ADCS GENERIC_ADCS_DI ST_Q_3").abs
    puts "sim42_q3 is #{sim42_Q3}"
    puts "adcs_q3 is #{adcs_Q3}"
    puts "st_q3 is #{st_Q3}"

    wait_check_expression("#{st_Q3} < #{sim42_Q3 + 0.05} or #{st_Q3} > #{sim42_Q3 - 0.05}", 30)
    wait_check_expression("#{st_Q3} < #{adcs_Q3 + 0.05} or #{st_Q3} > #{adcs_Q3 - 0.05}", 30)



    get_generic_star_tracker_hk()
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("GENERIC_STAR_TRACKER GENERIC_STAR_TRACKER_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
    
end

def confirm_adcs_data_loop()
    GENERIC_ADCS_DEVICE_LOOP_COUNT.times do |n|
        confirm_adcs_data()
    end
end
