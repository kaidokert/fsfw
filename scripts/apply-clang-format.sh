#!/bin/bash
if [[ ! -f README.md ]]; then
	cd ..
fi

cmake_fmt="cmake-format"
if command -v ${cmake_fmt} &> /dev/null; then
	cmake_fmt_cmd="${cmake_fmt} -i CMakeLists.txt"
	eval ${cmake_fmt_cmd}
fi

file_selectors="-iname *.h -o -iname *.cpp -o -iname *.c -o -iname *.tpp"
find ./src ${file_selectors} | xargs clang-format --style=file -i
find ./hal ${file_selectors} | xargs clang-format --style=file -i
find ./tests ${file_selectors} | xargs clang-format --style=file -i
