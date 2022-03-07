#!/bin/bash
if [[ ! -f README.md ]]; then
	cd ..
fi

find ./src -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i
find ./hal -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i
find ./tests -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format --style=file -i