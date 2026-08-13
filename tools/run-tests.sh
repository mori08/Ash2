#!/bin/bash
set -euo pipefail

REPO_ROOT=$(git rev-parse --show-toplevel)
EXE="$REPO_ROOT/Ash2/App/Ash2(debug).exe"

if [[ ! -f "$EXE" ]]; then
  echo "ERROR: 実行ファイルが見つかりません: $EXE"
  echo "先にビルドを実行してください: ./tools/build.sh"
  exit 1
fi

# このスクリプトはビルドを行わないため、古い exe を黙って実行してしまわないよう
# ソースとのタイムスタンプを比較する
STALE=$(find "$REPO_ROOT/Ash2/src" "$REPO_ROOT/Ash2/tests" "$REPO_ROOT/Ash2/Ash2.vcxproj" \
  -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.vcxproj' \) \
  -newer "$EXE" -print -quit)
if [[ -n "$STALE" ]]; then
  echo "ERROR: 実行ファイルがソースより古いです: $STALE"
  echo "先にビルドを実行してください: ./tools/build.sh"
  exit 1
fi

mkdir -p "$REPO_ROOT/logs"
LOG="$REPO_ROOT/logs/test.log"

echo "Running tests: $EXE"
# Siv3D の Main() は void で Catch2 の結果をプロセスの exit code に反映できないため、
# exe の exit code は無視し、Catch2 の出力から合否を判定する
ASH2_RUN_TESTS=1 "$EXE" 2>&1 | tee "$LOG" || true

if grep -q "All tests passed" "$LOG"; then
  exit 0
fi

echo "ERROR: テストが失敗しました（詳細は上記出力を参照）"
exit 1
