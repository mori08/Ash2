#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel)"
ASSET_DIR="$REPO_ROOT/Ash2/App/assets"
ASSET_LIST="$ASSET_DIR/asset_list"
RESOURCE_RC="$REPO_ROOT/Ash2/App/Resource.rc"

# asset_list を再生成
find "$ASSET_DIR" -type f | sort | while read -r file; do
  rel="${file#$REPO_ROOT/Ash2/App/}"
  [[ "$rel" == "assets/asset_list" ]] && continue
  echo "$rel"
done > "$ASSET_LIST"

# Resource.rc の App Resources セクションを更新
python3 - "$RESOURCE_RC" "$ASSET_LIST" <<'EOF'
import sys, re

rc_path, list_path = sys.argv[1], sys.argv[2]
with open(rc_path, encoding="utf-8") as f:
    content = f.read()
with open(list_path, encoding="utf-8") as f:
    assets = [line.strip() for line in f if line.strip()]

entries = "\n".join(f"Resource({a})" for a in assets)
section = (
    "//////////////////////////////////////////////////////\n"
    "//\n"
    "//\tSiv3D App Resources (Your application resources here)\n"
    "//\n"
    "//////////////////////////////////////////////////////\n\n"
    f"Resource(assets/asset_list)\n"
    f"{entries}\n"
)
new_content = re.sub(
    r"/{54}\n//\n//\tSiv3D App Resources.*",
    section,
    content,
    flags=re.DOTALL,
)
with open(rc_path, "w", encoding="utf-8") as f:
    f.write(new_content)
print("Updated:", rc_path)
print("Updated:", list_path)
EOF
