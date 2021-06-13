#!/bin/bash

set -e

pushd "$(dirname "$0")" >/dev/null

cd programs

for p in "$@"; do
    echo "$p"
    cd "$p"

    rm -rf build
    mkdir build
    cd build

    if [[ -f "../CMakeLists.txt" ]]; then
        cmake .. >/dev/null 2>/dev/null
        make >/dev/null 2>/dev/null
        exe="./main"
    else
        exe="../run.sh"
    fi

    if [ -z "$(ls .. | grep test_*)" ]; then
        cd ..
        cd ..
        continue
    fi

    for t in ../test_*; do
        t_out="$t/out"
        bt="$(basename "$t")"
        echo "- $bt"
        mkdir "$bt"
        b_out="$bt/out"
        mkdir "$b_out"

        if [[ -f "$t/prepare.sh" ]]; then
            "$t/prepare.sh"
        fi
        args="$(cat "$t/parameters.txt")"
        set +e
        ../../../../build/my_sandbox $exe "$args" --config="$t/config.yaml" --log="$b_out/log.txt" >"$b_out/stdout.txt" 2>"$b_out/stderr.txt"
        if [[ -f "$t/cleanup.sh" ]]; then
            "$t/cleanup.sh"
        fi
        set -e
        diff "$t_out/log.txt" "$b_out/log.txt"
        diff "$t_out/stdout.txt" "$b_out/stdout.txt"
        diff "$t_out/stderr.txt" "$b_out/stderr.txt"
    done

    cd ..
    cd ..
done

cd ..

popd >/dev/null
