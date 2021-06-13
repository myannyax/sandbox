#!/bin/bash

set -e

pushd "$(dirname "$0")" >/dev/null

"./test.sh" memory_limit memory_limit_child \
            niceness \
            permissions_read permissions_write \
            signals signals_child \
            stackoverflow stackoverflow_child \
            timeout timeout_child \
            timeout_cpu timeout_cpu_child timeout_cpu_prlimit timeout_cpu_signal_handler \
            move

popd > /dev/null
