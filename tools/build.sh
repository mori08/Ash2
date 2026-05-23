#!/bin/bash
set -euo pipefail

REPO_ROOT=$(git rev-parse --show-toplevel)

SYNC_ASSETS=true
while [[ $# -gt 0 ]]; do
  case "$1" in
    --no-sync-assets) SYNC_ASSETS=false; shift ;;
    *) shift ;;
  esac
done

if $SYNC_ASSETS; then
  bash "$REPO_ROOT/tools/sync-assets.sh"
fi

VSWHERE="C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
MSBUILD=$("$VSWHERE" -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | tr -d '\r' | head -1)

if [[ -z "$MSBUILD" ]]; then
  echo "ERROR: MSBuild が見つかりません。Visual Studio 2022 がインストールされているか確認してください。"
  exit 1
fi

mkdir -p "$REPO_ROOT/logs"

"$MSBUILD" "$REPO_ROOT/Ash2/Ash2.sln" \
  -p:Configuration=Debug \
  -p:Platform=x64 \
  -m \
  -v:minimal \
  -nologo \
  -fl "-flp:logfile=$REPO_ROOT/logs/build.log;verbosity=detailed"

echo "Build SUCCEEDED"
