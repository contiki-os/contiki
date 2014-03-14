#!/bin/bash
#
# This file checks your git index and runs uncrustify on every changed *.c and *.h
# file.
#
# Author: Valentin Sawadski <valentin@sawadski.eu>

# Exit if any called command exits nonzero
set -e

# Determine the repository root as the path returned by porcelain always
# start there
GIT_REPOSITORY_ROOT=$(git rev-parse --show-toplevel)

# This is the format command that will be run.
FORMAT_COMMAND=$GIT_REPOSITORY_ROOT/tools/code-style/uncrustify-fix-style.sh

# Parse all the files we get from git.
RAW_FILES=$(git status --porcelain)
# Set \n to be the field separator
IFS=$'\n'
# This splits the input by newline
GIT_FILES_BY_LINE=( $RAW_FILES )

# Now go throug them once more and remove the GIT status logs.
# Count the amount of C and H files we have to format.
IFS=$' '
for i in "${!GIT_FILES_BY_LINE[@]}"; do
                LINE_BY_SPACE=( ${GIT_FILES_BY_LINE[i]} )
                FILE=${LINE_BY_SPACE[1]}

                if [[ ${FILE: -2} = ".c" || ${FILE: -2} = ".h" ]]; then
                                # echo Formatting File: $FILE
                                $FORMAT_COMMAND $GIT_REPOSITORY_ROOT/$FILE
                fi
done

