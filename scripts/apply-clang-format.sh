#!/bin/bash
if [[ ! -f README.md ]]; then
	cd ..
fi

cmake_fmt="cmake-format"
if command -v ${cmake_fmt} &> /dev/null; then
	cmake_fmt_cmd="${cmake_fmt} -i CMakeLists.txt"
	eval ${cmake_fmt_cmd}
fi

find ./src -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i
find ./hal -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i
find ./tests -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i
