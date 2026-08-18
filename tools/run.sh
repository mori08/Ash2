#!/bin/bash
set -euo pipefail

REPO_ROOT=$(git rev-parse --show-toplevel)
EXE="$REPO_ROOT/Ash2/App/Ash2(debug).exe"
CRASH_LOG="$REPO_ROOT/Ash2/App/crash.log"

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

# 残っている記録を今回のものと取り違えないよう、起動前に削除する
rm -f "$CRASH_LOG"

echo "Launching: $EXE"
STATUS=0
"$EXE" || STATUS=$?

if [[ $STATUS -ne 0 ]]; then
  echo "ERROR: アプリが異常終了しました（終了コード: $STATUS）"
  if [[ -f "$CRASH_LOG" ]]; then
    cat "$CRASH_LOG"
  fi
  exit "$STATUS"
fi
