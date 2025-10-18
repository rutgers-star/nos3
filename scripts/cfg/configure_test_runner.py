import os
import shutil
import xml.etree.ElementTree as ET

# Component to XML mapping
components = {
    "arducam_test": "cam",
    "generic_adcs_test": "adcs",
    "generic_css_test": "css",
    "generic_eps_test": "eps",
    "generic_fss_test": "fss",
    "generic_imu_test": "imu",
    "generic_mag_test": "mag",
    "generic_radio_test": "radio",
    "generic_rw_test": "rw",
    "generic_st_test": "st",
    "generic_thruster_test": "thruster",
    "generic_torquer_test": "torquer",
    "novatel_oem615_test": "gps",
}

def clean_test_runner_targets(input_file, output_file, sc_root):
    """Remove REQUIRE_UTILITY lines for disabled components"""
    with open(input_file, 'r') as f:
        lines = f.readlines()

    lines_to_remove = set()

    for comp_key, xml_name in components.items():
        component_upper = comp_key.upper()
        node = sc_root.find(f"components/{xml_name}/enable")
        enabled = node is not None and node.text.strip().lower() == 'true'

        if enabled:
            continue

        print(f"[REMOVE] {comp_key} disabled — removing REQUIRE_UTILITY lines.")
        lines_to_remove.update({
            f"REQUIRE_UTILITY '{comp_key}'"
        })

    # Filter out unwanted lines
    filtered_lines = [line for line in lines if line.strip() not in lines_to_remove]

    with open(output_file, 'w') as f:
        f.writelines(filtered_lines)

def main():
    # === Parse XML to get SC config ===
    mission_file = 'nos3-mission.xml'
    mission_path = "./cfg/build/temp_mission/" + os.path.basename(mission_file)
    
    try:
        mission_tree = ET.parse(mission_path)
        mission_root = mission_tree.getroot()
        sc_cfg = mission_root.find("sc-1-cfg").text
        sc_cfg_path = './cfg/' + sc_cfg
        sc_tree = ET.parse(sc_cfg_path)
        sc_root = sc_tree.getroot()
    except Exception as e:
        print(f"Error parsing XML: {e}")
        return

    # === File paths ===
    input_path = './gsw/cosmos/config/tools/test_runner/stash/test_runner.txt'
    output_path = './gsw/cosmos/config/tools/test_runner/test_runner.txt'

    shutil.copyfile(input_path, output_path)

    clean_test_runner_targets(output_path, output_path, sc_root)

if __name__ == '__main__':
    main()