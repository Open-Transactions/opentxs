#!/bin/bash

set -e

SRC="${1}"
PRESET="${2}"
WORK="/home/output"

if [ ! -d "${SRC}" ]; then
    echo "Source tree missing. Mount opentxs source directory at ${SRC}"
    exit 1
fi

if [ ! -d "${WORK}" ]; then
    echo "Work directory missing. Mount build directory at ${WORK}"
    exit 1
fi

rm -rf "${WORK}/"*
/usr/bin/cmake \
    -S "${SRC}" \
    -B "${WORK}" \
    --preset "${PRESET}" \
    -DOT_BUNDLED_SECP256K1=OFF
/usr/bin/cmake \
    --build "${WORK}" \
    -- -k 0 \
    | tee "${WORK}/iwyu.txt"
cd "${SRC}"
/usr/bin/fix_includes.py --blank_lines --nocomments --nosafe_headers --reorder < "${WORK}/iwyu.txt"

output=$(/usr/bin/git --no-pager diff)

if [ -z "$output" ]; then
  echo "IWYU check successful"

  exit 0
else
  echo "IWYU error(s) found:"
  echo "${output}"

  exit 1
fi
