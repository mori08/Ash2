#!/usr/bin/env python3
import json
import subprocess
import sys

result = subprocess.run(
    ["git", "log", "main..HEAD", "--oneline"],
    capture_output=True,
    text=True,
    cwd=subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        capture_output=True, text=True
    ).stdout.strip()
)

count = len([l for l in result.stdout.splitlines() if l.strip()])

if count > 1:
    print(json.dumps({
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "additionalContext":
                f"警告: main..HEAD に {count} 件のコミットがあります。"
                "GIT.md のルール: 軽微な修正は直前のコミットに squash してから push すること。"
        }
    }))

sys.exit(0)
