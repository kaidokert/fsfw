#!/usr/bin/env python3
# -*- coding: utf-8 -*
"""Small portable helper script to generate LCOV HTML coverage data"""
import os
import platform
import sys
import time
import argparse
import webbrowser
from typing import List


"""Copy this helper script into your project folder. It will try to determine a CMake build folder
and then attempt to build your project with coverage information.

See Unittest documentation at https://egit.irs.uni-stuttgart.de/fsfw/fsfw for more
information how to set up the build folder.
"""
def main():

    parser = argparse.ArgumentParser(description="Processing arguments for LCOV helper script.")
    parser.add_argument(
        '-o', '--open', action='store_true', help='Open coverage data in webbrowser'
    )
    args = parser.parse_args()

    build_dir_list = []
    if not os.path.isfile('README.md'):
        os.chdir('..')
    for directory in os.listdir("."):
        if os.path.isdir(directory):
            os.chdir(directory)
            check_for_cmake_build_dir(build_dir_list)
            os.chdir("..")

    if len(build_dir_list) == 0:
        print("No valid CMake build directory found. Trying to set up hosted build")
        build_directory = 'build-Debug-Host'
        os.mkdir(build_directory)
        os.chdir(build_directory)
        os.system('cmake -DFSFW_OSAL=host -DFSFW_BUILD_UNITTESTS=ON ..')
        os.chdir('..')
    elif len(build_dir_list) == 1:
        build_directory = build_dir_list[0]
    else:
        print("Multiple build directories found!")
        build_directory = determine_build_dir(build_dir_list)
    perform_lcov_operation(build_directory)
    if os.path.isdir('fsfw-tests_coverage') and args.open:
        webbrowser.open('fsfw-tests_coverage/index.html')


def check_for_cmake_build_dir(build_dir_dict: list):
    if os.path.isfile("CMakeCache.txt"):
        build_dir_dict.append(os.getcwd())


def perform_lcov_operation(directory):
    os.chdir(directory)
    os.system("cmake --build . -- fsfw-tests_coverage -j")


def determine_build_dir(build_dir_list: List[str]):
    build_directory = ""
    for idx, directory in enumerate(build_dir_list):
        print(f"{idx + 1}: {directory}")
    while True:
        idx = input("Pick the directory to perform LCOV HTML generation by index: ")
        if not idx.isdigit():
            print("Invalid input!")
            continue

        idx = int(idx)
        if idx > len(build_dir_list) or idx < 1:
            print("Invalid input!")
            continue
        build_directory = build_dir_list[idx - 1]
        break
    return build_directory


if __name__ == "__main__":
    main()
