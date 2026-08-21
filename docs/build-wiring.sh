#!/usr/bin/env bash
# Re-render docs/wiring.pdf from docs/wiring.html.
#
# The HTML is the source of truth — edit that, then run this. Chrome is used
# because it honours the @page and print-colour rules in the stylesheet; Preview
# and textutil both throw the layout away.
set -euo pipefail
cd "$(dirname "$0")"

CHROME="/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
[ -x "$CHROME" ] || CHROME="/Applications/Brave Browser.app/Contents/MacOS/Brave Browser"
[ -x "$CHROME" ] || { echo "No Chrome or Brave found." >&2; exit 1; }

"$CHROME" --headless=new --disable-gpu --no-pdf-header-footer \
  --run-all-compositor-stages-before-draw --virtual-time-budget=4000 \
  --print-to-pdf="$PWD/wiring.pdf" "file://$PWD/wiring.html" 2>/dev/null

echo "wrote $PWD/wiring.pdf"
