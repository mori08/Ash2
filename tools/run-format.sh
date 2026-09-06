#!/bin/bash
# clang-format で整形する
#
# 使い方:
#   bash tools/run-format.sh src/foo.cpp src/bar.hpp

set -euo pipefail

: "${CLANG_FORMAT:?環境変数 CLANG_FORMAT が設定されていません。docs/SETUP.md「clang-format / clang-tidy」を参照してください}"

FILES=()
while [[ $# -gt 0 ]]; do
    FILES+=("$1")
    shift
done

if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "使い方: bash tools/run-format.sh <ファイル...>" >&2
    exit 1
fi

echo "=== clang-format ==="
"$CLANG_FORMAT" -i "${FILES[@]}"
echo "完了"
