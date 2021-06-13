#!/bin/bash

set -e

pushd "$(dirname "$0")" >/dev/null

"./test.sh" memory_limit memory_limit_child \
            niceness \
            permissions_read permissions_write \
            signals signals_child \
            timeout timeout_child

popd > /dev/null
