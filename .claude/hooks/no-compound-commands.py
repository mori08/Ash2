#!/usr/bin/env python3
import json
import re
import sys

data = json.load(sys.stdin)
command = data.get("tool_input", {}).get("command", "")

is_compound = re.search(r"(&&|\|\||;)", command)
is_git_or_gh = re.search(r"\b(git|gh)\b", command)

if is_compound and is_git_or_gh:
    print(json.dumps({
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "deny",
            "permissionDecisionReason":
                "git・ghコマンドでは複合コマンド(&&, ||, ;)を使わないでください。各コマンドを個別に実行してください。"
        }
    }))

sys.exit(0)
