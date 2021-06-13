#!/bin/bash

set -e

pushd "$(dirname "$0")" >/dev/null

"./test.sh" memory_limit permissions_read permissions_write

popd > /dev/null
