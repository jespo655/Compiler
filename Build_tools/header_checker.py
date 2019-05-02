
"""

For all header files (recognized by .h extension), recursively from the target folder, do this:

* Generate test file:
    #include "file.h"
    int main() { return 0; }
* Compile, piping the output to an output log file

Then check for errors. No compilation errors or warnings should be present.

"""

import os
import subprocess
import re
import ast
import time
import sys
import getopt

VERSION = "1.0.0"

# default options
build_folder = "BUILD"
src_folder = "."
compiler = "g++"
flag = "-std=gnu++14"
max_errors = 0
single_header = None
clean_check = False
verbose_output = False
quiet_output = False

# non-optional constants
timestamps_file = "header_timestamps.dat"
temp_file = "_tmp.cpp"
temp_output = "_tmp.exe"

# file compilation return codes
ERROR = 2
CHECKED_OK = 1
SKIPPED = 0


def check_file(header, timestamps, temp_file_path, temp_output_path):

    # check timestamp
    timestamp = time.ctime(os.path.getmtime(header))
    if (header in timestamps):
        if (timestamp == timestamps[header]):
            if (verbose_output): print("Skipping file - not changed: {}".format(header))
            return SKIPPED
    timestamps[header] = timestamp

    # create temp file
    write_data(temp_file_path, '#include "{}"\n{}'.format(header.replace("{}\\".format(src_folder), ""), "int main() { return 0; }"))

    # compile
    if (not quiet_output): print("Checking {}...".format(header))
    sys.stdout.flush()
    error = subprocess.call([compiler, "-{}".format(flag), "-I", src_folder, temp_file_path, "-o", temp_output_path])

    if (error):
        timestamps[header] = None
        return ERROR
    else:
        return CHECKED_OK



def check_files():
    if (not quiet_output): print("Checking files...")
    start = time.time()

    timestamps_path = "{}/{}".format(build_folder, timestamps_file)
    temp_file_path = "{}/{}".format(build_folder, temp_file)
    temp_output_path = "{}/{}".format(build_folder, temp_output)

    timestamps = {}
    if os.path.exists(build_folder):
        if (not clean_check): timestamps = read_data(timestamps_path, {})
    else:
        os.makedirs(build_folder)

    h_files = []
    if (single_header):
        h_files.append(os.path.join(src_folder, single_header))
    else:
        for root, dirs, files in os.walk(src_folder):
            h_files.extend([os.path.join(root, fi) for fi in files if fi.endswith(".h")])

    files_checked = 0
    files_skipped = 0
    files_error = 0
    for file in h_files:
        result = check_file(file, timestamps, temp_file_path, temp_output_path)
        if (result == ERROR):
            files_checked += 1
            files_error += 1
        elif (result == CHECKED_OK):
            files_checked += 1
        elif (result == SKIPPED):
            files_skipped += 1
        if (max_errors > 0 and files_error >= max_errors): break

    write_data(timestamps_path, timestamps)

    if os.path.exists(temp_file_path): os.remove(temp_file_path)
    if os.path.exists(temp_output_path): os.remove(temp_output_path)

    end = time.time()
    print("Checked {} files and skipped {} files in {:.1f} seconds. Found errors in {}/{} files.".format(files_checked, files_skipped, end - start, files_error, files_skipped+files_checked))





def write_data(filename, data):
    s = str(data);
    with open(filename, 'wb') as file:
        file.write(s.encode("utf-8"))

def read_data(filename, default=None):
    try:
        with open(filename, 'rb') as file:
            return ast.literal_eval(file.read().decode("utf-8"))
    except FileNotFoundError:
        return default




def main(argv):

    global build_folder, src_folder, compiler, flag, clean_check, verbose_output, quiet_output, max_errors, single_header

    helpstring = """
Checks header files for compilation errors.
All header files should be self contained, and not generate any errors when compiled alone.
Headers that compile without errors and that haven't changed are skipped.

Usage: py {} [--help] [--version] [options]

    -b, --build <folder>    set build folder (default: {})
    -s, --src <folder>      set source folder (default: {})
    -h, --header <filename> set to only check one file (default: not set)
    -k, --compiler          set compiler (default: {})
    -f, --flag              set compilation flag (default: {})
    -e, --maxerr <#>        set the maximum number of files with errors that are accepted before the script terminates
    -c, --clean             make a clean check, ignoring previous results
    -v, --verbose           make additional output
    -q, --quiet             make minimal output (overrides --verbose)""".format(__file__, build_folder, src_folder, compiler, flag)

    try:
        opts, args = getopt.getopt(argv, "hb:s:k:h:f:ce:vq", ["help", "version", "build=", "src=", "header=", "maxerr", "compiler=", "flag=", "clean", "verbose", "quiet"])
    except getopt.GetoptError:
        print("{}".format(helpstring))
        sys.exit(2)

    for opt, arg in opts:
        if opt in ["-h", "--help"]:
            print("{}".format(helpstring))
            sys.exit()

        if opt in ["--version"]:
            print("{}".format(VERSION))
            sys.exit()

        elif opt in ["-b", "--build"]:
            build_folder = arg

        elif opt in ["-s", "--src"]:
            src_folder = arg

        elif opt in ["-k", "--compiler"]:
            compiler = arg

        elif opt in ["-f", "--flag"]:
            flag = arg

        elif opt in ["-h", "--header"]:
            single_header = arg

        elif opt in ["-e", "--maxerr"]:
            max_errors = int(arg)

        elif opt in ["-c", "--clean"]:
            clean_check = True

        elif opt in ["-v", "--verbose"]:
            verbose_output = True

        elif opt in ["-q", "--quiet"]:
            quiet_output = True

    if (quiet_output): verbose_output = False

    check_files()


if __name__ == '__main__':
    main(sys.argv[1:])




