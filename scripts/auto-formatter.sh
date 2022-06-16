#!/bin/bash
if [[ ! -f README.md ]]; then
  cd ..
fi

cmake_fmt="cmake-format"
file_selectors="-iname CMakeLists.txt"
if command -v ${cmake_fmt} &> /dev/null; then
  ${cmake_fmt} -i CMakeLists.txt
  find ./src ${file_selectors} | xargs ${cmake_fmt}  -i
else
  echo "No ${cmake_fmt} tool found, not formatting CMake files"
fi

cpp_format="clang-format"
file_selectors="-iname *.h -o -iname *.cpp -o -iname *.c -o -iname *.tpp"
if command -v ${cpp_format} &> /dev/null; then
  find ./src ${file_selectors} | xargs ${cpp_format} --style=file -i
  find ./hal ${file_selectors} | xargs ${cpp_format} --style=file -i
  find ./tests ${file_selectors} | xargs ${cpp_format} --style=file -i
else
  echo "No ${cpp_format} tool found, not formatting C++/C files"
fi
