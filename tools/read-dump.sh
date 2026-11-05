#!/bin/bash
# logs/crash.dmp を cdb で開き、シンボルの読み込み状況と指定のコマンドの結果を出力する
# 使い方: ./tools/read-dump.sh ['<cdb コマンド>']（省略時はスタックの上から10フレーム）
set -euo pipefail

REPO_ROOT=$(git rev-parse --show-toplevel)
DUMP="$REPO_ROOT/logs/crash.dmp"
PDB_DIR="$REPO_ROOT/Ash2/Intermediate/Ash2/Debug"
CDB=cdbX64.exe  # WinDbg の実行エイリアス（本体の cdb.exe はバージョン付きフォルダにある）

COMMANDS=${1:-k 10}

if [[ ! -f "$DUMP" ]]; then
  echo "ERROR: ダンプが見つかりません: $DUMP"
  exit 1
fi

# cdb の出力は CP932 のため UTF-8 に変換する
# 自前の PDB は srv* より先に探させる（srv* で見つかるとシンボルキャッシュにコピーされ、ビルドごとに溜まる）
# NatVis の読み込み・解放の記録は調査に使わないため除外する
"$CDB" -z "$DUMP" -y "$PDB_DIR;srv*" \
  -c "ld Ash2_debug_; lm m Ash2_debug_; .lines -e; $COMMANDS; q" \
  2>&1 | iconv -f CP932 -t UTF-8 | grep -v 'NatVis script'
