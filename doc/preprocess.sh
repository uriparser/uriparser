#! /usr/bin/env bash
# Run GCC preprocessor and delete empty lines
: ${CC:=cc}
PS4='doc/preprocess.sh|# '
set -x
"${CC}" -E -DURI_DOXYGEN -DURI_NO_UNICODE -C -I ../include "$1" | sed -e '/^$/d' -e 's/COMMENT_HACK//g'
