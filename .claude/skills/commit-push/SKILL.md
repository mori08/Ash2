---
name: commit-push
description: Stage files, commit with proper message, and push to remote
---

Before doing anything, check the current branch with git branch. If on main:
- Read docs/GIT.md to confirm whether the work qualifies as `chore` (direct commit allowed) or `enhancement`/`bug` (must use a feature/fix branch and PR). If it is not `chore`, stop and tell the user to create a branch first.

Run git status and git diff in parallel to understand what has changed.

Then stage the relevant files, commit, and push using these rules:
- Commit message: English, imperative mood, no prefix (e.g. "Add X", "Fix Y", "Update Z")
- Always include a related issue reference in the commit message (e.g. `#3` to reference, `close #3` to auto-close on merge)
- Always push immediately after committing
- Stage specific files by name, not `git add -A`
- Append to commit message: Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>

If the user provided a message with $ARGUMENTS, use it as-is. Otherwise draft a message from the diff.
