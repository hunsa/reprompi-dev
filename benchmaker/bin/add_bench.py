#! /usr/bin/env python
import re
import sys
import os
import shutil
from optparse import OptionParser

# in bin
base_path = os.path.dirname( os.path.realpath( sys.argv[0] ) )
#cd ..
base_path = os.path.dirname( base_path )
lib_path = os.path.join( base_path, "lib" )
sys.path.append(lib_path)

from code_parser import CodeParser


def copy_dir_structure(input_dir, output_dir, override=False):

    print(f"\nstart copying {input_dir}")
    flist = os.listdir(input_dir)
    for f in flist:
        src = os.path.join(input_dir, f)
        dst = os.path.join(output_dir, f)

        if os.path.isdir(src):
            if not override:
                print(f"skipping existing dir {dst}")
                sys.exit("exiting. Use -f to force overridding files")
            else:
                if os.path.exists(dst):
                    os.rmdir(dst)
                shutil.copytree(src, dst)

        elif os.path.isfile(src):
            if not override:
                print(f"skipping existing file {dst}")
                sys.exit("exiting. Use -f to force overridding files")
            else:
                if os.path.exists(dst):
                    os.unlink(dst)
                shutil.copyfile(src, dst)
    print(f"copying of {input_dir} done\n")


def has_reprompi_benchcode(fname):
    found = False
    # check if line starts with //@
    pat = re.compile("^\s*\/\/\s*@.*")
    with open(fname) as f:
        content = f.readlines()
        for l in content:
            if re.match(pat, l):
                #print(l, end="")
                found = True
                break
    return found


def find_reprompi_files(dir):
    res_files = []

    for dirpath, dirs, files in os.walk(dir):
        for f in files:
            if f.endswith(".c") and f not in res_files:
                fpath = os.path.join(dirpath, f)
                print(fpath, has_reprompi_benchcode(fpath))
                if has_reprompi_benchcode(fpath):
                    res_files.append(fpath)

    return res_files


if __name__ == "__main__":

    parser = OptionParser(usage="usage: %prog [options]")

    parser.add_option("-d", "--expdir",
                       action="store",
                       dest="input_dir",
                       type="string",
                       help="directory of input files")
    parser.add_option("-o", "--outputdir",
                       action="store",
                       dest="output_dir",
                       type="string",
                       help="output directory")
    parser.add_option("-f", "--force",
                      action="store_true",
                      dest="override",
                      help="override output files if they exist")
    
    (options, args) = parser.parse_args()

    if options.input_dir is None:
        parser.print_help()
        print("Error: Experiment directory not specified")
    input_dir = os.path.abspath(options.input_dir)

    if options.output_dir is None:
        parser.print_help()
        sys.exit("Error: Output directory not specified.\n")
    output_dir = os.path.abspath(options.output_dir)
        
    if not os.path.exists(options.output_dir):
        print("Creating output directory: %s" % (options.output_dir))
        os.makedirs(options.output_dir)

    copy_dir_structure(input_dir, output_dir, options.override)

    reprompi_files = find_reprompi_files(output_dir)

    print("reprompi_files", reprompi_files)
    for rf in reprompi_files:
        print(f"converting {rf}")
        cp = CodeParser(rf)
        mod_content = cp.generate_output_file()
        #print("mod_content", mod_content)
        with open(rf, "w") as fh:
            fh.writelines(mod_content)

    # codegen = BenchmarkCodeGen(input_dir, output_dir)
    # codegen.generate_benchmarking_code()

    print("Done.\nGenerated code can be found here: %s" % output_dir)
    
        