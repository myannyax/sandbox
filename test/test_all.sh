#!/bin/bash

set -e

pushd "$(dirname "$0")" >/dev/null

"./test.sh" memory_limit memory_limit_child \
            niceness \
            permissions_read permissions_write \
            signals signals_child \
            timeout timeout_child \
            timeout_cpu \
            stackoverflow stackoverflow_child

popd > /dev/null
