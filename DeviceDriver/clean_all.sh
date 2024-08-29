#!/bin/bash

# 현재 디렉토리 및 하위 디렉토리에서 Makefile을 찾고 make clean 실행
find . -type f -name Makefile | while read -r makefile_path; do
    dir=$(dirname "$makefile_path")
    echo "Running 'make clean' in $dir"
    make -C "$dir" clean
done

