---
name: commit-push
description: Stage files, commit with proper message, and push to remote
---

Before doing anything, read docs/GIT.md and check the current branch with git branch. Run these in parallel.

If on main:
- Confirm whether the work qualifies as `chore` (direct commit allowed) or `enhancement`/`bug` (must use a feature/fix branch and PR). If it is not `chore`, stop and tell the user to create a branch first.

If on a feature/fix branch:
- Run `git log main..HEAD --oneline` to check the number of commits ahead of main.
- Apply the commit organization rules from GIT.md: if there are multiple commits, determine whether they should be squashed or kept separate, and tell the user your assessment before proceeding.

Run git status and git diff in parallel to understand what has changed.

Then stage the relevant files, commit, and push using these rules:
- Commit message: English, imperative mood, no prefix (e.g. "Add X", "Fix Y", "Update Z")
- Always include a related issue reference in the commit message (e.g. `#3` to reference, `close #3` to auto-close on merge)
- Always push immediately after committing
- Stage specific files by name, not `git add -A`
- Append to commit message: Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>

If the user provided a message with $ARGUMENTS, use it as-is. Otherwise draft a message from the diff.
