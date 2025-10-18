'''

Description: calls C++ SYNOPSIS library by passing input files

'''

import subprocess
import os
import json

def synopsis(path_to_synopsis_cli, asdpdb_file, rule_config_file, similarity_config_file, output_file=None):
    '''
    Entry point to C++ implementation of SYNOPSIS (synopsis_cli)

    downlink states (given in dictionaries describing each DP):
    UNTRANSMITTED = 0
    TRANSMITTED = 1
    DOWNLINKED = 2

    Parameters
    ----------
    path_to_synopsis_cli: str
        Path to the CLI executable `synopsis_cli`
    asdpdb_file: str
        Path to DB file
    rule_config_file: str
        Path to JSON file with rules
    similarity_config_file: str
        Path to JSON file with similarity and diversity parameters
    output_file: str
        Path to JSON file to output, or None if writing to file is not desired

    Returns
    -------
    logs: list
        List of log statements from SYNOPSIS
    prioritized_dps: list
        None if there is an error running SYNOPSIS, else a list of dictionaries holding DP information in order of priority 
    
    

    '''

    if not os.path.exists(asdpdb_file):
        print("invalid asdpdb_file")
    if not os.path.exists(rule_config_file):
        print("invalid rule_config_file")
    if not os.path.exists(similarity_config_file):
        print("invalid similarity_config_file")

    logs = None
    prioritized_dps = None

    try:
        if output_file is not None:
            print("output file given: ", output_file)
            cp = subprocess.run(
                [path_to_synopsis_cli, asdpdb_file, rule_config_file, similarity_config_file, output_file],
                capture_output=True
            )
        else:
            print("no output file given")
            cp = subprocess.run(
                [path_to_synopsis_cli, asdpdb_file, rule_config_file, similarity_config_file],
                capture_output=True
            )

    except:
        print("Error running SYNOPSIS, check synopsis_cli.cpp")

    else:
        if cp.returncode == 0:
            print("SYNOPSIS ran successfully")
        else:
            print("SYNOPSIS returned error")

        stddout = cp.stdout.decode().split('\n')
        stderr = cp.stderr.decode().split('\n')

        logs = []
        msg = []
        for line in stddout:
            if "[INFO]" in line or "[WARN]" in line or "[ERROR]" in line:
                logs.append(line)
            else:
                msg.append(line)
        logs.extend(stderr)
        
        msg = (' '.join(msg))
        msg = json.loads(msg)
        prioritized_dps = [msg[str(i)] for i in msg['prioritized_list']]

    finally:
        return logs, prioritized_dps


if __name__ == "__main__":

    
    path_to_synopsis_cli = './build/synopsis_cli'

    # Example files
    asdpdb_file = 'test/data/dd_example.db'
    rule_config_file = 'test/data/dd_example_rules.json'
    similarity_config_file = 'test/data/dd_example_similarity_config.json'
    # CLI call: ./build/synopsis_cli test/data/dd_example.db test/data/dd_example_rules.json test/data/dd_example_similarity_config.json output.json

    # Pass name of JSON File to output to, or None if writing to file is not desired
    output_file = 'output.json'  # or None if outputs are not to be saved
    # output_file = None

    logs, prioritized_dps = synopsis(path_to_synopsis_cli, asdpdb_file, rule_config_file, similarity_config_file, output_file)

    print("prioritized dps: ")
    for i in prioritized_dps:
        print(i)

    print("logs: ")
    for line in logs:
        print(line)
