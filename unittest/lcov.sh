#!/bin/bash
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory _coverage
