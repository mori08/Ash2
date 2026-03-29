---
name: commit-push
description: Stage files, commit with proper message, and push to remote
---

Run git status and git diff in parallel to understand what has changed.

Then stage the relevant files, commit, and push using these rules:
- Commit message: English, imperative mood, no prefix (e.g. "Add X", "Fix Y", "Update Z")
- Always push immediately after committing
- Stage specific files by name, not `git add -A`
- Append to commit message: Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>

If the user provided a message with $ARGUMENTS, use it as-is. Otherwise draft a message from the diff.
