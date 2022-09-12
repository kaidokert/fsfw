#!/usr/bin/env python3
# -*- coding: utf-8 -*
"""Small portable helper script to generate test or doc configuration for the
flight software framework
"""
import os
import argparse
import webbrowser
import shutil
import sys
import time
from shutil import which
from typing import List


UNITTEST_FOLDER_NAME = "cmake-build-tests"
DOCS_FOLDER_NAME = "build-docs"


def main():

    parser = argparse.ArgumentParser(description="FSFW helper script")
    choices = ("docs", "tests")
    parser.add_argument(
        "type", metavar="type", choices=choices, help=f"Target type. Choices: {choices}"
    )
    parser.add_argument(
        "-a", "--all", action="store_true", help="Create, build and open specified type"
    )
    parser.add_argument(
        "-c",
        "--create",
        action="store_true",
        help="Create docs or test build configuration",
    )
    parser.add_argument(
        "-b", "--build", action="store_true", help="Build the specified type"
    )
    parser.add_argument(
        "-o",
        "--open",
        action="store_true",
        help="Open test or documentation data in webbrowser",
    )
    parser.add_argument(
        "-v",
        "--valgrind",
        action="store_true",
        help="Run valgrind on generated test binary",
    )
    parser.add_argument(
        "-g",
        "--generators",
        default = "Ninja",
        action="store",
        help="CMake generators",
    )
    parser.add_argument(
        "-w",
        "--windows",
        default=False,
        action="store_true",
        help="Run on windows",
    )

    args = parser.parse_args()
    if args.all:
        args.build = True
        args.create = True
        args.open = True
    elif not args.build and not args.create and not args.open and not args.valgrind:
        print(
            "Please select at least one operation to perform. "
            "Use helper.py -h for more information"
        )
        sys.exit(1)

    # This script can be called from root and from script folder
    if not os.path.isfile("README.md"):
        os.chdir("..")
    build_dir_list = []
    if not args.create:
        build_dir_list = build_build_dir_list()

    if args.type == "tests":
        handle_tests_type(args, build_dir_list)
    elif args.type == "docs":
        handle_docs_type(args, build_dir_list)
    else:
        print("Invalid or unknown type")
        sys.exit(1)


def handle_docs_type(args, build_dir_list: list):
    if args.create:
        if os.path.exists(DOCS_FOLDER_NAME):
            shutil.rmtree(DOCS_FOLDER_NAME)
        create_docs_build_cfg()
        build_directory = DOCS_FOLDER_NAME
    elif len(build_dir_list) == 0:
        print(
            "No valid CMake docs build directory found. Trying to set up docs build system"
        )
        shutil.rmtree(DOCS_FOLDER_NAME)
        create_docs_build_cfg()
        build_directory = DOCS_FOLDER_NAME
    elif len(build_dir_list) == 1:
        build_directory = build_dir_list[0]
    else:
        print("Multiple build directories found!")
        build_directory = determine_build_dir(build_dir_list)
    os.chdir(build_directory)
    if args.build:
        cmd_runner("cmake --build . -j")
    if args.open:
        if not os.path.isfile("docs/sphinx/index.html"):
            # try again..
            cmd_runner("cmake --build . -j")
            if not os.path.isfile("docs/sphinx/index.html"):
                print(
                    "No Sphinx documentation file detected. "
                    "Try to build it first with the -b argument"
                )
                sys.exit(1)
        webbrowser.open("docs/sphinx/index.html")


def handle_tests_type(args, build_dir_list: list):
    if args.create:
        if os.path.exists(UNITTEST_FOLDER_NAME):
            shutil.rmtree(UNITTEST_FOLDER_NAME)
        create_tests_build_cfg(args)
        build_directory = UNITTEST_FOLDER_NAME
    elif len(build_dir_list) == 0:
        print(
            "No valid CMake tests build directory found. "
            "Trying to set up test build system"
        )
        create_tests_build_cfg(args)
        build_directory = UNITTEST_FOLDER_NAME
    elif len(build_dir_list) == 1:
        build_directory = build_dir_list[0]
    else:
        print("Multiple build directories found!")
        build_directory = determine_build_dir(build_dir_list)
    os.chdir(build_directory)
    if args.build:
        perform_lcov_operation(build_directory, False)
    if args.open:
        if not os.path.isdir("fsfw-tests_coverage"):
            print(
                "No Unittest folder detected. Try to build them first with the -b argument"
            )
            sys.exit(1)
        webbrowser.open("fsfw-tests_coverage/index.html")
    if args.valgrind:
        if which("valgrind") is None:
            print("Please install valgrind first")
            sys.exit(1)
        cmd_runner("valgrind --leak-check=full ./fsfw-tests")
        os.chdir("..")


def create_tests_build_cfg(args):
    os.mkdir(UNITTEST_FOLDER_NAME)
    os.chdir(UNITTEST_FOLDER_NAME)
    if args.windows:
        cmake_cmd = 'cmake -G "' + args.generators + '" -DFSFW_OSAL=host -DFSFW_BUILD_TESTS=ON \
        -DGCOVR_PATH="py -m gcovr" ..'
    else:
        cmake_cmd = 'cmake -G "' + args.generators + '" -DFSFW_OSAL=host -DFSFW_BUILD_TESTS=ON ..'
    cmd_runner(cmake_cmd)
    os.chdir("..")


def create_docs_build_cfg():
    os.mkdir(DOCS_FOLDER_NAME)
    os.chdir(DOCS_FOLDER_NAME)
    cmd_runner("cmake -DFSFW_OSAL=host -DFSFW_BUILD_DOCS=ON ..")
    os.chdir("..")


def build_build_dir_list() -> list:
    build_dir_list = []
    for directory in os.listdir("."):
        if os.path.isdir(directory):
            os.chdir(directory)
            build_dir_list = check_for_cmake_build_dir(build_dir_list)
            os.chdir("..")
    return build_dir_list


def check_for_cmake_build_dir(build_dir_list: list) -> list:
    if os.path.isfile("CMakeCache.txt"):
        build_dir_list.append(os.getcwd())
    return build_dir_list


def perform_lcov_operation(directory: str, chdir: bool):
    if chdir:
        os.chdir(directory)
    cmd_runner("cmake --build . -- fsfw-tests_coverage -j")


def determine_build_dir(build_dir_list: List[str]):
    build_directory = ""
    for idx, directory in enumerate(build_dir_list):
        print(f"{idx + 1}: {directory}")
    while True:
        idx = input("Pick the directory: ")
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


def cmd_runner(cmd: str):
    print(f"Executing command: {cmd}")
    os.system(cmd)


if __name__ == "__main__":
    main()
