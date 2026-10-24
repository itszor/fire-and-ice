#!/bin/sh
# Build Fire & Ice from your own copy of the game:
#     ./build.sh "Fire & Ice (1995)(Renegade)(Disk 1 of 2).adf" "...(Disk 2 of 2).adf"
# then: cd recomp && ./fireice
# The steps are recipe.json's "build", run by SuperSpider's tools/weave.py.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
SUPERSPIDER="${SUPERSPIDER:-$HOME/stuff/superspider}"
[ $# -gt 0 ] || { echo "usage: $0 DISK1.adf DISK2.adf" >&2; exit 2; }
(cd "$SUPERSPIDER" && dune build)
WEAVE_RECIPES="$HERE" exec python3 -I "$SUPERSPIDER/tools/weave.py" build "$@"
