#!/bin/bash
if [[ ! -f README.md ]]; then
  cd ..
fi

folder_list=(
  "./src"
  "./unittests"
)

cmake_fmt="cmake-format"
file_selectors="-iname CMakeLists.txt"
if command -v ${cmake_fmt} &> /dev/null; then
  ${cmake_fmt} -i CMakeLists.txt
  find ./src ${file_selectors} | xargs ${cmake_fmt} -i
  find ./unittests ${file_selectors} | xargs ${cmake_fmt} -i
else
  echo "No ${cmake_fmt} tool found, not formatting CMake files"
fi

cpp_format="clang-format"
file_selectors="-iname *.h -o -iname *.cpp -o -iname *.c -o -iname *.tpp"
if command -v ${cpp_format} &> /dev/null; then
  for dir in ${folder_list[@]}; do
    echo "Auto-formatting ${dir} recursively"
    find ${dir} ${file_selectors} | xargs clang-format --style=file -i
  done
else
  echo "No ${cpp_format} tool found, not formatting C++/C files"
fi
