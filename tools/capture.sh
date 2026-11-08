#!/bin/bash
# 指定のシナリオセクションから起動し、指定の秒数で画面を 1 枚撮影して終了する
# 使い方: tools/capture.sh <section> <秒>
# 標準出力には保存した PNG のパスだけを出す（経過とエラーは標準エラー出力）
set -euo pipefail

if [[ $# -ne 2 ]]; then
  echo "使い方: $0 <section> <秒>" >&2
  exit 1
fi

SECTION=$1
SECONDS_AFTER=$2

if [[ -z "$SECTION" ]]; then
  echo "ERROR: セクション名が空です" >&2
  exit 1
fi
if [[ ! "$SECONDS_AFTER" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
  echo "ERROR: 秒数は 0 以上の数値で指定してください: $SECONDS_AFTER" >&2
  exit 1
fi

REPO_ROOT=$(git rev-parse --show-toplevel)
SCREENSHOT_DIR="$REPO_ROOT/Ash2/App/screenshots"

# run-cdb.sh は起動時に PNG を消し、ゲームの終了まで戻らない
# 失敗時は set -e により run-cdb.sh の終了コードのまま終わる
ASH2_START_SECTION="$SECTION" ASH2_CAPTURE_AFTER="$SECONDS_AFTER" \
  "$REPO_ROOT/tools/run-cdb.sh" >&2

# 撮影中に PrintScreen が押された場合に備えて最新のものを選ぶ
LATEST=$(ls -t "$SCREENSHOT_DIR"/*.png 2>/dev/null | head -n 1 || true)
if [[ -z "$LATEST" ]]; then
  echo "ERROR: スクリーンショットが保存されていません（撮影前にウィンドウが閉じられた可能性があります）" >&2
  exit 1
fi
echo "$LATEST"
