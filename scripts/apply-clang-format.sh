#!/bin/bash
if [[ ! -f README.md ]]; then
	cd ..
fi

cmake_fmt="cmake-format"
if command -v ${cmake_fmt} &> /dev/null; then
	cmake_fmt_cmd="${cmake_fmt} -i CMakeLists.txt"
	eval ${cmake_fmt_cmd}
else
	echo "No cmake-format tool found, not formatting CMake files"
fi

cpp_format="clang-format"
if command -v ${clang-format} &> /dev/null; then
	find ./src -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i
	find ./hal -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i
	find ./tests -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i
else
	echo "No clang-format tool found, not formatting C++/C files"
fi