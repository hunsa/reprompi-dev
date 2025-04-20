import config as cfg
import os
import argparse
from datetime import datetime


if __name__ == "__main__":

    # parse command line argument for dir to new experiment
    parser = argparse.ArgumentParser(description="Setting up new pgchecker experiment.")
    parser.add_argument("-d", "--dir", type=str, required=True, help="Base directory for experiments")
    parser.add_argument("-N", "--num_nodes", type=int, default=1, help="Number of nodes to use for the experiment")
    parser.add_argument("-n", "--ppn", type=int, default=1, help="Number of processes per node to use for the experiment")

    args = parser.parse_args()
    new_exp_dir = args.dir

    # check if the directory exists
    if not os.path.exists(new_exp_dir):
        # exit if it does not exist
        print(f"Base experiment directory {new_exp_dir} does not exist. Exiting.")
        exit(1)

    # create a new directory for the experiment
    now = datetime.now()
    # format the date and time
    dt_string = now.strftime("%Y%m%d_%H%M%S")
    # create the new directory name
    exp_dir = "pgexp_" + dt_string
    # create the new directory
    new_exp_dir = os.path.join(new_exp_dir, exp_dir)
    os.makedirs(new_exp_dir, exist_ok=False)
    print(f"Creating new experiment directory {new_exp_dir}")

    # create subdirectories in new_exp_dir
    exp_config_dir = os.path.join(new_exp_dir, cfg.PG_EXP_CONFIG_DIR)
    os.makedirs(exp_config_dir, exist_ok=False)
    exp_data_dir = os.path.join(new_exp_dir, cfg.PG_EXP_DATA_DIR)
    os.makedirs(exp_data_dir, exist_ok=False)

    # execute pgmpi_info to get supported collectives
    pgmpi_info_exe = os.path.join(cfg.REPOS["pgmpitunelib"]["INSTALL_DIR"], "bin", "pgmpi_info")
    # execute the command and save outout in exp_config_dir
    pgmpi_info_out = os.path.join(exp_config_dir, "pgmpi_info.csv")
    pgmpi_info_cmd = f"{pgmpi_info_exe} > {pgmpi_info_out}"
    print(f"Executing {pgmpi_info_cmd}")
    os.system(pgmpi_info_cmd)
    print(f"pgmpi_info output saved in {pgmpi_info_out}")

    # create the config file for the experiment
    pgchecker_config_file = os.path.join(exp_config_dir, "input.txt")
    with open(pgchecker_config_file, "a") as fh:
        for e in cfg.PG_COLLECTIVES_EXP:
            fh.write(e + "\n")
    print(f"pgchecker input file created in {pgchecker_config_file}")

    print("Run experiment with pgchecker like so:")
    # PGCHECKER_CSV="./exp/pgexp_20250418_135112/config/pgmpi_info.csv" mpirun -np 2 ./build/bin/pgchecker -f ./exp/pgexp_20250418_135112/config/input.txt -o ./exp/pgexp_20250418_135112/data
    print(f"PGCHECKER_CSV={pgmpi_info_out} srun -N {args.num_nodes} --ntasks-per-node={args.ppn} ./build/bin/pgchecker -f {pgchecker_config_file} -o {exp_data_dir}")
    



