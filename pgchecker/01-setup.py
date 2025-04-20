import os
import sys

import config as cfg


def checkout_and_build(repo):

    # Check if the repository is already cloned
    if not os.path.exists(repo["CHECKOUT_DIR"]):
        os.makedirs(cfg.PG_BUILD_DIR, exist_ok=True)
        print(f"Cloning {repo["REPO_GIT"]} into {repo["CHECKOUT_DIR"]}...")
        os.chdir(cfg.PG_BUILD_DIR)
        os.system(f"git clone {repo["REPO_GIT"]} {repo["REPO_CHECKOUT_NAME"]}")
    else:
        print(f"Directory {repo["CHECKOUT_DIR"]} already exists. Skipping clone.")
    
    # Checkout the specific SHA1
    os.chdir(f"{repo["CHECKOUT_DIR"]}")
    os.system(f"git checkout {repo["REPO_SHA1"]}")

    # build the repository
    print(f"Building {repo["REPO_CHECKOUT_NAME"]}\n{repo["BUILD_CMD"]}")
    os.system(repo["BUILD_CMD"])


def build_dependencies():
    for dep in cfg.DEPENDENCIES:
        print(f"Building {dep}...")
        repo = cfg.REPOS[dep]
        checkout_and_build(repo)

def build_pgchecker():
    
    print(f"Building pgchecker in {cfg.PG_BUILD_DIR}, installing into {cfg.PG_INSTALL_DIR}")
    print(cfg.PG_BUILD_CMD)
    os.system(cfg.PG_BUILD_CMD)


if __name__ == "__main__":

    print("building pgchecker...")
    print("pgtuner build path: ", cfg.PG_BUILD_DIR)
    
    build_dependencies()
    build_pgchecker()
        
        

