#! /usr/bin/env bash
# Run GCC preprocessor and delete empty lines
set -e -u -o pipefail
: ${CC:=cc}
PS4='doc/preprocess.sh|# '
set -x
{
    if [[ "${1}" =~ \.txt$ ]]; then
        cat "${1}"
    else
        "${CC}" -E -DURI_DOXYGEN -DURI_NO_UNICODE -C -I ../include "$1"
    fi
} | sed -e '/^$/d' -e 's/COMMENT_HACK//g'
