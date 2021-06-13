#!/bin/bash

set -e

pushd "$(dirname "$0")" >/dev/null

cd ..

rm -rf build
mkdir build
cd build

cmake .. >/dev/null 2>/dev/null
make >/dev/null 2>/dev/null

cd ..

cd test
cd programs

for p in *; do
    echo "$p"
    cd "$p"

    rm -rf build
    mkdir build
    cd build

    cmake .. >/dev/null 2>/dev/null
    make >/dev/null 2>/dev/null

    for t in ../test_*; do
        t_out="$t/out"
        bt="$(basename "$t")"
        echo "- $bt"
        mkdir "$bt"
        b_out="$bt/out"
        mkdir "$b_out"

        args="$(cat "$t/parameters.txt")"
        set +e
        ../../../../build/my_sandbox ./main "$args" --config="$t/config.yaml" --log="$b_out/log.txt" >"$b_out/stdout.txt" 2>"$b_out/stderr.txt"
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
