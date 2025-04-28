#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status.
set -u # Treat unset variables as an error when substituting.
set -o pipefail # Causes a pipeline to return the exit status of the last command that exited with a non-zero status

PRODUCT_ID="cafe/6942"
DEVICE_PATTERN="/sys/class/tty/*/device/uevent"

echo "Searching for Sisyphus device..."

DEVICE_FILE=$(grep -ril "PRODUCT=${PRODUCT_ID}" ${DEVICE_PATTERN} | head -n 1 || true) # Allow grep to fail without exiting script yet

if [[ -z "$DEVICE_FILE" ]]; then
    echo "Error: Sisyphus device not found" >&2
    exit 1
fi

DEV_PATH=$(sed 's|/sys/class/tty/\(.*\)/device/uevent|/dev/\1|' <<< "$DEVICE_FILE")

if [ -z "$DEV_PATH" ]; then
  echo "Error: Sisyphus device not found" >&2
  exit 1
fi

# Check if it's a character device AFTER confirming it's not empty
if [ ! -c "$DEV_PATH" ]; then
  echo "Error: Not a Sisyphus device" >&2
  exit 1
fi

echo "Found device: ${DEV_PATH}"
echo "Sending reset command..." >&2

# Send the reset sequence
if ! printf "reset\x04" > "${DEV_PATH}"; then
    echo "Error: Failed to write reset command to ${DEV_PATH}" >&2
    exit 1
fi

sleep 1
exit 0 # Explicit success exit
