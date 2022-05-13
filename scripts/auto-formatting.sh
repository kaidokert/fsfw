#!/bin/bash
if [[ ! -f README.md ]]; then
  cd ..
fi

cmake_fmt="cmake-format"
if command -v ${cmake_fmt} &> /dev/null; then
  cmake_fmt_cmd="${cmake_fmt} -i CMakeLists.txt"
  eval ${cmake_fmt_cmd}
else
  echo "No ${cmake_fmt} tool found, not formatting CMake files"
fi

cpp_format="clang-format"
file_selectors="-iname *.h -o -iname *.cpp -o -iname *.c -o -iname *.tpp"
if command -v ${cpp_format} &> /dev/null; then
  find ./src ${file_selectors} | xargs clang-format --style=file -i
  find ./hal ${file_selectors} | xargs clang-format --style=file -i
  find ./tests ${file_selectors} | xargs clang-format --style=file -i
else
  echo "No ${cpp_format} tool found, not formatting C++/C files"
fi
