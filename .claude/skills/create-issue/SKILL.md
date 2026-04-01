---
name: create-issue
description: Create a GitHub issue with a label
---

Create a GitHub issue using the gh CLI.

$ARGUMENTS format: `<label> <title>` or just `<title>` (label optional)

Rules:
- Available labels: `bug`, `enhancement`, `chore`
- If the user provides a label as the first word (bug/enhancement/chore), use it. Otherwise infer the most appropriate label from the title/context.
- Title: use as-is if provided, otherwise ask the user
- No assignee
- Body: leave empty unless the user provides additional context

Command to run:
```
gh issue create --title "<title>" --label "<label>" --body ""
```

After creation, show the issue URL.
