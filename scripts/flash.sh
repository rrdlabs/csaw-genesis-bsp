#!/usr/bin/env bash
# Flash a merged CsAW Genesis image. Usage: scripts/flash.sh [port] [image]
# esptool v5 requires flash-mode/size/freq options AFTER the write-flash
# subcommand; the previous order failed with "No such option '--flash-mode'".
set -euo pipefail

PORT="${1:-/dev/ttyUSB0}"
IMAGE="${2:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/build/merged-binary.bin}"

if [[ ! -f "$IMAGE" ]]; then
    echo "error: $IMAGE not found - build a merged image first" >&2
    exit 2
fi
if [[ ! -e "$PORT" ]]; then
    echo "error: $PORT not found - connect the board and check the driver" >&2
    exit 3
fi

python -m esptool --chip esp32s3 --port "$PORT" -b 460800 \
    --before default-reset --after hard-reset \
    write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m \
    0x0 "$IMAGE"

echo "Flashed. Run 'idf.py -p $PORT monitor' and check the bringup log."
