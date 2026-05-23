#!/bin/bash
set -euo pipefail

REPO_ROOT=$(git rev-parse --show-toplevel)
EXE="$REPO_ROOT/Ash2/App/Ash2(debug).exe"

if [[ ! -f "$EXE" ]]; then
  echo "ERROR: 実行ファイルが見つかりません: $EXE"
  echo "先にビルドを実行してください: ./tools/build.sh"
  exit 1
fi

mkdir -p "$REPO_ROOT/logs"

echo "Launching: $EXE"
"$EXE" 2>&1 | tee "$REPO_ROOT/logs/runtime.log" || echo "WARNING: App exited with non-zero status"
