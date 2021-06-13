#!/bin/sh

set -e

pushd "$(dirname "$0")" >/dev/null

"./test.sh" memory_limit permissions_read

popd > /dev/null
