
"""

Recursively delete all files in specified build folder.

@TODO: only remove .o, .exe and .dat files, and give error if any other file is encountered

"""

import os
import sys
import getopt

VERSION = "1.0.0"

# default options
build_folder = "BUILD"
verbose_output = False
quiet_output = False


def clean():
    print("Removing build files")
    for root, dirs, files in os.walk(build_folder, topdown=False):
        for name in files:
            os.remove(os.path.join(root, name))
        for name in dirs:
            os.rmdir(os.path.join(root, name))
    if os.path.exists(build_folder):
        os.rmdir(build_folder)


def main(argv):

    global build_folder, src_folder, cpp_compiler, cpp_flag, c_compiler, c_flag, clean_build, verbose_output, quiet_output

    helpstring = """
Removes build files from directory.

Usage: py {} [--help] [--version] [options]

    -b, --build <folder>    set build folder (default: {})
    -v, --verbose           make additional output
    -q, --quiet             make minimal output (overrides --verbose)""".format(__file__, build_folder, VERSION)

    try:
        opts, args = getopt.getopt(argv, "hb:vq", ["help", "version", "build=", "verbose", "quiet"])
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

        elif opt in ["-v", "--verbose"]:
            verbose_output = True

        elif opt in ["-q", "--quiet"]:
            quiet_output = True

    if (quiet_output): verbose_output = False

    clean()


if __name__ == '__main__':
    main(sys.argv[1:])
