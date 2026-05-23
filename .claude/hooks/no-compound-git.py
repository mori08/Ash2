#!/usr/bin/env python3
import json
import re
import sys

data = json.load(sys.stdin)
command = data.get("tool_input", {}).get("command", "")

if re.search(r"\bgit\b", command) and re.search(r"(&&|\|\||;)", command):
    print(json.dumps({
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "deny",
            "permissionDecisionReason":
                "git コマンドに複合コマンド(&&, ||, ;)を使わないでください。各コマンドを個別に実行してください。"
        }
    }))

sys.exit(0)
