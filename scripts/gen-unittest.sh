#!/bin/sh
mkdir build-Unittest && cd build-Unittest
cmake -DFSFW_BUILD_UNITTESTS=ON -DFSFW_OSAL=host -DCMAKE_BUILD_TYPE=Debug ..
