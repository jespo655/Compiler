
"""

Recursively go through the target folder, looking for code files.

For each file found, the last changed timestamp against a timestamp in a database.
If it matches, skip the file. Otherwise, compile it to .o!

For each .c and .cpp file, check the changed timestamp against
    - .c file: compile with gcc -c "path/to/file.c" -o "build/file.o"
    - .cpp file: compile with g++ -c "path/to/file.c" -o "build/file.o"

Link with g++ build/* -o "build/outputname"

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
outputname = "output"
cpp_compiler = "g++"
cpp_flag = "std=gnu++14"
c_compiler = "gcc"
c_flag = ""
max_errors = 0
clean_build = False
verbose_output = False
quiet_output = False
libraries = []

# non-optional constants
timestamps_file = "timestamps.dat"
no_compiler = "none"

# file compilation return codes
ERROR = 2
BUILT_OK = 1
SKIPPED = 0


def compile_file(root, file, timestamps, cpp=True):

    # calculate input and output file paths
    input = os.path.join(root, file)
    output = os.path.join(root, "{}.o".format(file)).replace("\\", ".").replace("{}.".format(src_folder), "{}\\".format(build_folder))

    # check timestamp
    timestamp = time.ctime(os.path.getmtime(input))
    if (input in timestamps):
        if (timestamp == timestamps[input]):
            if (verbose_output): print("Skipping file - not changed: {}".format(input))
            return SKIPPED
    timestamps[input] = timestamp

    # get compile command
    command = None
    if cpp:
        command = [cpp_compiler]
        if (cpp_flag != ""): command.append("-{}".format(cpp_flag))
    else:
        command = [c_compiler]
        if (c_flag != ""): command.append("-{}".format(c_flag))
    command.extend(["-c", input, "-o", output])

    # compile
    if (not quiet_output): print("Compiling {}...".format(input))
    sys.stdout.flush()
    error = subprocess.call(command)
    if (error):
        timestamps[input] = None
        return ERROR
    else:
        return BUILT_OK



def build():

    if (clean_build):
        print("Removing old output files...")
        for root, dirs, files in os.walk(build_folder, topdown=False):
            for name in files:
                if name.endswith(".o"):
                    os.remove(os.path.join(root, name))

    print("Compiling files...")
    start = time.time()

    timestamps_path = "{}/{}".format(build_folder, timestamps_file)

    timestamps = {}
    if os.path.exists(build_folder):
        if (not clean_build): timestamps = read_data(timestamps_path, {})
    else:
        os.makedirs(build_folder)

    c_files = []
    cpp_files = []
    for root, dirs, files in os.walk(src_folder):
        if c_compiler != no_compiler: c_files.extend([[root, fi] for fi in files if fi.endswith(".c")])
        if cpp_compiler != no_compiler: cpp_files.extend([[root, fi] for fi in files if fi.endswith(".cpp")])

    errors = 0
    changed = False
    for root_file in c_files:
        res = compile_file(root_file[0], root_file[1], timestamps, cpp=False)
        if (res == ERROR): errors += 1
        if (res != SKIPPED): changed = True
        if (max_errors > 0 and errors >= max_errors): break

    for root_file in cpp_files:
        res = compile_file(root_file[0], root_file[1], timestamps, cpp=True)
        if (res == ERROR): errors += 1
        if (res != SKIPPED): changed = True
        if (max_errors > 0 and errors >= max_errors): break

    write_data(timestamps_path, timestamps)

    if errors:
        end = time.time()
        print("Finished with errors in {:.1f} seconds".format(end - start))

    elif changed:
        print("Linking...")
        output = os.path.join(build_folder, outputname)
        if (not output.endswith(".exe")): output = "{}.exe".format(output)
        command = ["g++", "{}\\*.o".format(build_folder), "-o", output]
        command.extend(libraries)
        sys.stdout.flush()
        error = subprocess.call(command)

        end = time.time()
        print("Completed in {:.1f} seconds".format(end - start))

    else:
        end = time.time()
        print("Completed in {:.1f} seconds - no changes.".format(end - start))


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

    global build_folder, src_folder, cpp_compiler, cpp_flag, c_compiler, c_flag, clean_build, verbose_output, quiet_output, max_errors, outputname

    helpstring = """
Builds all C and C++ files using incremental compilation.
Files that have been previously compiled without errors and haven't changed are skipped.

Usage: py {} [--help] [--version] [options]

    -b, --build <folder>    set build folder (default: {})
    -s, --src <folder>      set source folder (default: {})
    -o, --output <name>     set output name (default: {})
    -k, --cpp_compiler      set C++ compiler (default: {}) set to "none" to disable compilation of .cpp files.
    -f, --cpp_flag          set C++ compilation flag (default: {})
    -l, --c_compiler        set C compiler (default: {}) set to "none" to disable compilation of .c files.
    -g, --c_flag            set C compilation flag (default: {})
    -e, --maxerr <#>        set the maximum number of files with errors that are accepted before the script terminates
    -L, --library <path>    include external library in compilation
    -c, --clean             make a clean build, ignoring previous results
    -v, --verbose           make additional output
    -q, --quiet             make minimal output (overrides --verbose)""".format(__file__, build_folder, src_folder, outputname, cpp_compiler, cpp_flag, c_compiler, c_flag)

    try:
        opts, args = getopt.getopt(argv, "hb:s:k:f:l:g:L:o:e:cvq", ["help", "version", "build=", "src=", "maxerr", "output", "cpp_compiler=", "cpp_flag=", "c_compiler=", "c_flag=", "library=", "clean", "verbose", "quiet"])
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

        elif opt in ["-k", "--cpp_compiler"]:
            cpp_compiler = arg

        elif opt in ["-f", "--cpp_flag"]:
            cpp_flag = arg

        elif opt in ["-l", "--c_compiler"]:
            c_compiler = arg

        elif opt in ["-g", "--c_flag"]:
            c_flag = arg

        elif opt in ["-o", "--output"]:
            outputname = arg

        elif opt in ["-e", "--maxerr"]:
            max_errors = int(arg)

        elif opt in ["-L", "--library"]:
            libraries.append(arg)

        elif opt in ["-c", "--clean"]:
            clean_build = True

        elif opt in ["-v", "--verbose"]:
            verbose_output = True

        elif opt in ["-q", "--quiet"]:
            quiet_output = True

    if (quiet_output): verbose_output = False

    build()


if __name__ == '__main__':
    main(sys.argv[1:])
