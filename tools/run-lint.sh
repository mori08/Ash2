#!/bin/bash
# clang-format（整形）と clang-tidy（静的解析）をまとめて実行するラッパー
#
# 使い方:
#   bash tools/run-lint.sh src/foo.cpp src/bar.cpp
#   bash tools/run-lint.sh src/**/*.cpp
#   bash tools/run-lint.sh $(git diff --name-only --diff-filter=d HEAD | grep -E '\.(cpp|h)$')
#
# オプション:
#   --format-only   clang-format のみ実行
#   --tidy-only     clang-tidy のみ実行
#   --jobs N        clang-tidy の並列数（デフォルト: CPU コア数）

set -euo pipefail

# ---- 設定 ---------------------------------------------------------------

REPO=$(git rev-parse --show-toplevel)
PROJECT_WIN=$(cygpath -w "$REPO/Ash2")

# SIV3D_0_6_16 が未設定だと cygpath が空文字を展開してサイレントに誤パスになるため先にガード
: "${SIV3D_0_6_16:?環境変数 SIV3D_0_6_16 が設定されていません。Siv3D SDK のインストールを確認してください}"
SIV3D_WIN=$(cygpath -w "$SIV3D_0_6_16")

TIDY_WIN=$(cygpath -w "$REPO/Ash2/.tidy")
VCPKG_WIN=$(cygpath -w "$REPO/Ash2/vcpkg_installed/x64-windows/x64-windows/include")

CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
CLANG_TIDY="${CLANG_TIDY:-clang-tidy}"

RUN_FORMAT=1
RUN_TIDY=1
# nproc は Linux / MSYS2 では使えるが、Git for Windows の Git Bash には含まれない。
# Windows 組み込みの NUMBER_OF_PROCESSORS をフォールバックとして使う。
JOBS=$(nproc 2>/dev/null || echo "${NUMBER_OF_PROCESSORS:-4}")

# ---- 引数パース ---------------------------------------------------------

FILES=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --format-only) RUN_TIDY=0 ;;
        --tidy-only)   RUN_FORMAT=0 ;;
        --jobs)        JOBS="$2"; shift ;;
        *)             FILES+=("$1") ;;
    esac
    shift
done

if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "使い方: bash tools/run-lint.sh [--format-only|--tidy-only] [--jobs N] <ファイル...>" >&2
    exit 1
fi

# ---- clang-format -------------------------------------------------------

if [[ $RUN_FORMAT -eq 1 ]]; then
    echo "=== clang-format ==="
    "$CLANG_FORMAT" -i "${FILES[@]}"
    echo "完了"
    echo ""
fi

# ---- clang-tidy（並列実行 + 出力バッファ） ------------------------------

# clang-tidy は .cpp のみ対象（.hpp を直接渡すとコンテキスト不足で解析が不完全になる）
CPP_FILES=()
for f in "${FILES[@]}"; do
    [[ "$f" == *.cpp ]] && CPP_FILES+=("$f")
done

if [[ $RUN_TIDY -eq 1 ]] && [[ ${#CPP_FILES[@]} -eq 0 ]]; then
    echo "clang-tidy: .cpp ファイルがないためスキップ"
    RUN_TIDY=0
fi

if [[ $RUN_TIDY -eq 1 ]]; then
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
            --driver-mode=cl /std:c++latest /Zc:__cplusplus /utf-8 \
            "/FI${PROJECT_WIN}\\src\\stdafx.h" \
            -D_DEBUG -D_WINDOWS \
            -D_ENABLE_EXTENDED_ALIGNED_STORAGE \
            -D_SILENCE_CXX20_CISO646_REMOVED_WARNING \
            -D_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS \
            -D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS \
            -DUSE_TEST \
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
        job_count=$(( job_count + 1 ))  # (( job_count++ )) は set -e 下で 0 のとき exit 1 になるため使わない

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
fi
