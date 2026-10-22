#!/usr/bin/env bash
# docs/ is canonical (GitHub Pages serves branch main, folder /docs).
# This script regenerates the duplicate copies from docs/:
#   docs/sim/index.html -> webapp/public/index.html   (identical copy)
#   docs/index.html     -> index.html (repo root)     (copy with sim/ links rewritten to docs/sim/)
# Usage: scripts/sync_web.sh          write the copies
#        scripts/sync_web.sh --check  exit 1 if any copy is out of date (no writes)
# Remember: NEUROFLY_VERSION in both pages must match /VERSION.
set -euo pipefail
cd "$(dirname "$0")/.."

gen_root() { sed -e 's#"sim/index.html"#"docs/sim/index.html"#g' -e 's#"kit_box/index.html"#"docs/kit_box/index.html"#g' docs/index.html; }

check=0
[ "${1:-}" = "--check" ] && check=1

rc=0
if [ $check -eq 1 ]; then
  cmp -s docs/sim/index.html webapp/public/index.html || { echo "OUT OF SYNC: webapp/public/index.html"; rc=1; }
  gen_root | cmp -s - index.html || { echo "OUT OF SYNC: index.html"; rc=1; }
  ver=$(tr -d '[:space:]' < VERSION)
  for f in docs/index.html docs/sim/index.html; do
    grep -q "NEUROFLY_VERSION = '$ver'" "$f" || { echo "VERSION MISMATCH: $f != VERSION ($ver)"; rc=1; }
  done
  [ $rc -eq 0 ] && echo "web copies in sync (v$ver)"
  exit $rc
fi

mkdir -p webapp/public
cp docs/sim/index.html webapp/public/index.html
gen_root > index.html
echo "synced: webapp/public/index.html, index.html"
