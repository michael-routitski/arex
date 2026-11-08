#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

./bt.sh -p -r

if [[ "$EUID" -eq 0 ]]; then
  install -m 755 build/dev/arex /usr/local/bin/arex
else
  sudo install -m 755 build/dev/arex /usr/local/bin/arex
fi

printf 'Installed arex to /usr/local/bin/arex\n'