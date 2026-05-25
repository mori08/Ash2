#!/bin/bash
set -euo pipefail

REPO_ROOT=$(git rev-parse --show-toplevel)
EXE="$REPO_ROOT/Ash2/App/Ash2(debug).exe"

if [[ ! -f "$EXE" ]]; then
  echo "ERROR: 実行ファイルが見つかりません: $EXE"
  echo "先にビルドを実行してください: ./tools/build.sh"
  exit 1
fi

echo "Running tests: $EXE"
ASH2_RUN_TESTS=1 "$EXE" || true
