#!/bin/bash
# clang-tidy で静的解析を実行する（.cpp のみ対象）
#
# 使い方:
#   bash tools/run-tidy.sh src/foo.cpp src/bar.cpp
#   bash tools/run-tidy.sh src/**/*.cpp
#   bash tools/run-tidy.sh $(git diff --name-only --diff-filter=d HEAD | grep -E '\.cpp$')
#
# オプション:
#   --jobs N   並列数（デフォルト: CPU コア数）

set -euo pipefail

# ---- 設定 ---------------------------------------------------------------

REPO=$(git rev-parse --show-toplevel)
PROJECT_WIN=$(cygpath -w "$REPO/Ash2")

: "${SIV3D_0_6_16:?環境変数 SIV3D_0_6_16 が設定されていません。Siv3D SDK のインストールを確認してください}"
SIV3D_WIN=$(cygpath -w "$SIV3D_0_6_16")

TIDY_WIN=$(cygpath -w "$REPO/Ash2/.tidy")
VCPKG_WIN=$(cygpath -w "$REPO/Ash2/vcpkg_installed/x64-windows/x64-windows/include")

CLANG_TIDY="${CLANG_TIDY:-clang-tidy}"
JOBS=$(nproc 2>/dev/null || echo "${NUMBER_OF_PROCESSORS:-4}")

# ---- 引数パース ---------------------------------------------------------

FILES=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --jobs) JOBS="$2"; shift ;;
        *)      FILES+=("$1") ;;
    esac
    shift
done

if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "使い方: bash tools/run-tidy.sh [--jobs N] <ファイル...>" >&2
    exit 1
fi

# .cpp のみ対象
CPP_FILES=()
for f in "${FILES[@]}"; do
    [[ "$f" == *.cpp ]] && CPP_FILES+=("$f")
done

if [[ ${#CPP_FILES[@]} -eq 0 ]]; then
    echo "clang-tidy: .cpp ファイルがないためスキップ"
    exit 0
fi

# ---- clang-tidy（並列実行 + 出力バッファ） ------------------------------

echo "=== clang-tidy (並列数: $JOBS) ==="

TMPDIR_TIDY=$(mktemp -d)
trap 'rm -rf "$TMPDIR_TIDY"' EXIT

TIDY_FAILED=0

run_tidy_one() {
    local src="$1"
    local outfile="$2"
    local target_win
    target_win=$(cygpath -w "$src")

    MSYS_NO_PATHCONV=1 MSYS2_ARG_CONV_EXCL="*" \
    "$CLANG_TIDY" \
        "--header-filter=.*Ash2[\\/](src|tests)[\\/].*" \
        "$target_win" \
        -- \
        --driver-mode=cl /std:c++latest /Zc:__cplusplus /utf-8 /EHsc \
        "/FI${PROJECT_WIN}\\src\\stdafx.h" \
        -D_DEBUG -D_WINDOWS \
        -D_ENABLE_EXTENDED_ALIGNED_STORAGE \
        -D_SILENCE_CXX20_CISO646_REMOVED_WARNING \
        -D_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS \
        -D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS \
        "-I${TIDY_WIN}" "-I${PROJECT_WIN}\\src" "-I${PROJECT_WIN}" \
        -imsvc "${SIV3D_WIN}\\include" \
        -imsvc "${SIV3D_WIN}\\include\\ThirdParty" \
        -imsvc "$VCPKG_WIN" \
        > "$outfile" 2>&1
}

flush_batch() {
    for i in "${!pids[@]}"; do
        local outfile="$TMPDIR_TIDY/$(echo "${file_map[$i]}" | tr '/' '_').out"
        if ! wait "${pids[$i]}"; then
            TIDY_FAILED=1
        fi
        echo "=== ${file_map[$i]} ==="
        cat "$outfile"
        echo ""
    done
    pids=()
    file_map=()
    job_count=0
}

job_count=0
pids=()
file_map=()

for src in "${CPP_FILES[@]}"; do
    outfile="$TMPDIR_TIDY/$(echo "$src" | tr '/' '_').out"
    run_tidy_one "$src" "$outfile" &
    pids+=($!)
    file_map+=("$src")
    job_count=$(( job_count + 1 ))

    if [[ $job_count -ge $JOBS ]]; then
        flush_batch
    fi
done

flush_batch  # 残り

if [[ $TIDY_FAILED -ne 0 ]]; then
    echo "clang-tidy: 警告またはエラーがあります" >&2
    exit 1
else
    echo "clang-tidy: 問題なし"
fi
