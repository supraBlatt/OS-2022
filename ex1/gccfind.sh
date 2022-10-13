#!/bin/bash

# gccfind.sh [ path ] [ word ] [ -r , is optional ]
makefile () {
  cat << EOF
%.out: %.c
	@gcc -w -o \$@ \$<
EOF
}

# check inputs
if [[ "$#" -lt 2 ]]; then 
    echo "Not enough parameters"
    exit 1
fi

if [[ "$#" -gt 3 ]]; then
    echo "Too many parameters"
    exit 1
fi 

# check flag input 
case $3 in 
    "")
        recurse="-maxdepth 1"
        ;;
    "-r")
        recurse=""
        ;;
    *)
        echo "bad flag"
        exit 1
        ;;
esac

root="$1"
search="$2"


# find and remove all .out files 
find "$root" $recurse -type f -name "*.out" -delete

# recompile all .c files that contain $word 
find "$root" $recurse -type f -name "*.c" \
    | xargs grep -lwi "$search" \
    | sed s/"\.c$"/".out"/g \
    | xargs make -j8 -f <(makefile) 

