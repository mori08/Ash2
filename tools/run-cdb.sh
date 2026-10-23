#!/bin/bash
# ゲームを cdb の下で起動し、クラッシュしたらその瞬間のフルダンプを残す
set -euo pipefail

REPO_ROOT=$(git rev-parse --show-toplevel)
EXE="$REPO_ROOT/Ash2/App/Ash2(debug).exe"
PDB_DIR="$REPO_ROOT/Ash2/Intermediate/Ash2/Debug"
CDB=cdbX64.exe  # WinDbg の実行エイリアス（本体の cdb.exe はバージョン付きフォルダにある）

LOG_DIR="$REPO_ROOT/logs"
DUMP="$LOG_DIR/crash.dmp"

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

mkdir -p "$LOG_DIR"
rm -f "$DUMP"

# 止まったときの処理（全イベント共通）
ON_CRASH=".lastevent; .dump /ma /o $DUMP; q"

CONFIG=$(mktemp)
trap 'rm -f "$CONFIG"' EXIT

cat > "$CONFIG" <<EOF
sxi out
sxi ld
sxe -c "$ON_CRASH" av
sxe -c "$ON_CRASH" dz
sxe -c "$ON_CRASH" bpe
sxe -c "$ON_CRASH" eh
sxe -c "$ON_CRASH" -c2 "$ON_CRASH" sbo
sxe -c "$ON_CRASH" sov
sxe -c "$ON_CRASH" ii
sxe -c "$ON_CRASH" *
g
q
EOF

echo "Launching under cdb: $EXE"
# cdb の出力は CP932 のため UTF-8 に変換する
# mktemp は /tmp/... 形式を返すため、cdb には Windows のパスに直して渡す
# 自前の PDB は srv* より先に探させる（srv* で見つかるとシンボルキャッシュにコピーされ、ビルドごとに溜まる）
"$CDB" -G -y "$PDB_DIR;srv*" -cf "$(cygpath -m "$CONFIG")" "$EXE" 2>&1 | iconv -f CP932 -t UTF-8

if [[ -f "$DUMP" ]]; then
  echo "ERROR: クラッシュしました。ダンプ: $DUMP"
  exit 1
fi
echo "正常に終了しました"
