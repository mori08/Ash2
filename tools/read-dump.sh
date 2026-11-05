#!/bin/bash
# logs/crash.dmp を cdb で開き、クラッシュの要約または指定のコマンドの結果を出力する
# 使い方: ./tools/read-dump.sh ['<cdb コマンド>']
set -euo pipefail

REPO_ROOT=$(git rev-parse --show-toplevel)
DUMP="$REPO_ROOT/logs/crash.dmp"
PDB_DIR="$REPO_ROOT/Ash2/Intermediate/Ash2/Debug"
CDB=cdbX64.exe  # WinDbg の実行エイリアス（本体の cdb.exe はバージョン付きフォルダにある）

# 深い再帰でも呼び出し元まで辿れるよう、cdb の上限まで取る
MAX_FRAMES=0xffff

# 指定のコマンドを実行し、その結果だけを出力する
run_cdb() {
  # cdb の出力は CP932 のため UTF-8 に変換する
  # コマンドの誤りで q まで届かなくても入力待ちで止まらないよう、標準入力を閉じる
  "$CDB" -z "$DUMP" -y "$SYMBOL_PATH" \
    -c "ld Ash2_debug_; .lines -e; .echo @@@; $1; q" < /dev/null 2>&1 \
    | iconv -f CP932 -t UTF-8 | tr -d '\r' | grep -v 'NatVis script' \
    | sed -n '/^@@@$/,$p' | sed '1d; /^quit:$/d'
}

# 例外オブジェクトのメッセージを出力する
print_exception_message() {
  local object=$1 types=$2 value=""
  if [[ $types == *"s3d::Error"* ]]; then
    # 短い文字列は std::u32string のオブジェクト内に置かれ、長さで読む場所が変わる
    local str="((Ash2_debug_!s3d::Error*)0x$object)->m_what.m_string._Mypair._Myval2"
    value=$(run_cdb "dx -r0 $str._Myres > 3 ? $str._Bx._Ptr : (char32_t*)$str._Bx._Buf,s32")
  elif [[ $types == *"std::exception"* ]]; then
    value=$(run_cdb "dx -r0 ((Ash2_debug_!std::exception*)0x$object)->_Data._What,s")
  fi
  # dx の出力は「式 : アドレス : "値" [Type: 型]」の形で、値がないときは引用符が付かない
  if [[ $value == *': "'* ]]; then
    value=${value#*: \"}
    echo "[message] ${value%\" \[Type:*}"
  fi
}

# assert の式と場所を出力する
print_assert() {
  local frame=$1 locals
  locals=$(run_cdb ".frame $frame; dv /t")
  sed -n 's/^wchar_t \* expression = [^ ]* "\(.*\)"$/[assert] \1/p' <<< "$locals"
  local file line
  file=$(sed -n 's/^wchar_t \* file_name = [^ ]* "\(.*\)"$/\1/p' <<< "$locals")
  line=$(sed -n 's/^unsigned int line_number = \(.*\)$/\1/p' <<< "$locals")
  echo "[assert_at] ${file##*\\}:$((line))"
}

# kn の出力から番地を除き、名前と場所を短くする
# 同じ行の連続は1行にまとめ、Main より下の起動処理は出力しない
format_stack() {
  awk -v repo="$REPO_ROOT/" '
    function shorten(name,    i, c, depth, out) {
      gsub(/`anonymous namespace\x27::/, "", name)
      depth = 0; out = ""
      for (i = 1; i <= length(name); i++) {
        c = substr(name, i, 1)
        if (depth == 0 && c == "<" && substr(name, i, 8) == "<lambda_") {
          while (c != ">") { out = out c; i++; c = substr(name, i, 1) }
          out = out c
        } else if (c == "<" && out ~ /operator$/) {
          out = out c
        } else if (c == "<") {
          if (depth == 0) out = out "<...>"
          depth++
        } else if (c == ">" && depth > 0) {
          depth--
        } else if (depth == 0) {
          out = out c
        }
      }
      return out
    }
    function location(source,    path, at) {
      gsub(/\\/, "/", source)
      at = index(source, " @ ")
      path = substr(source, 1, at - 1)
      if (index(path, repo) == 1) path = substr(path, length(repo) + 1)
      else sub(/^.*\//, "", path)
      return path ":" substr(source, at + 3)
    }
    function flush() {
      if (count == 0) return
      print last_number " " last (count > 1 ? " (" count " 回連続)" : "")
      count = 0
    }
    NR == 1 { next }
    {
      number = $1
      line = $0
      sub(/^[0-9a-f]+ +(\(Inline Function\) +[-`]+|[0-9a-f`]+) +[-0-9a-f`]+ +/, "", line)
      sub(/^Ash2_debug_!/, "", line)
      if (match(line, / \[[^]]* @ [0-9]+\]$/)) {
        text = shorten(substr(line, 1, RSTART - 1))
        sub(/\+0x[0-9a-f]+$/, "", text)
        text = text "  [" location(substr(line, RSTART + 2, RLENGTH - 3)) "]"
      } else {
        text = shorten(line)
      }
      if (count > 0 && text == last) { count++; next }
      flush()
      last = text; last_number = number; count = 1
      if (line ~ /^Main(\+| )/) exit
    }
    END { flush() }
  '
}

if [[ ! -f "$DUMP" ]]; then
  echo "ERROR: ダンプが見つかりません: $DUMP"
  exit 1
fi

# 不一致のまま srv* を探すと、最新の PDB がシンボルキャッシュにコピーされ、ビルドごとに溜まる
# そのため自前の PDB だけで照合し、一致してから srv* を使う
SYMBOL_PATH=$PDB_DIR
SYMBOLS=$(run_cdb 'lm m Ash2_debug_')
if [[ $SYMBOLS != *"(private pdb symbols)"* ]]; then
  echo "ERROR: ダンプがこのビルドの PDB と一致しません。再現し直してください"
  echo "$SYMBOLS"
  exit 1
fi
SYMBOL_PATH="$PDB_DIR;srv*"

if [[ $# -gt 0 ]]; then
  run_cdb "$1"
  exit 0
fi

RECORD=$(run_cdb '.exr -1')
sed -n 's/^ *ExceptionCode: \(.*\)$/[event] \1/p; s/^Attempt to \(.*\)$/[detail] \1/p' <<< "$RECORD"
TYPES=$(sed -n 's/^ *Type *: \(class \|struct \)\?\(.*\)$/\2/p' <<< "$RECORD")
if [[ -n $TYPES ]]; then
  echo "[type] $(head -n 1 <<< "$TYPES")"
  OBJECT=$(sed -n 's/^ *pExceptionObject: *\(.*\)$/\1/p' <<< "$RECORD")
  print_exception_message "$OBJECT" "$TYPES"
fi

STACK=$(run_cdb "kn $MAX_FRAMES")
ASSERT_FRAME=$(sed -n 's/^\([0-9a-f]*\) .*!_w\?assert+.*$/\1/p' <<< "$STACK")
if [[ -n $ASSERT_FRAME ]]; then
  print_assert "$ASSERT_FRAME"
fi

echo "[stack]"
format_stack <<< "$STACK"
