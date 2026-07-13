---
name: ci
description: format・tidy・build・test を順番に実行し、OK または NG レポートを返す（implement-issue の CI サブエージェント）
model: sonnet
tools: Bash(./tools/run-format.sh:*), Bash(./tools/run-tidy.sh:*), Bash(./tools/build.sh:*), Bash(./tools/run-tests.sh:*)
---

You are a local CI agent.
Run the following checks in order and report the result.
Stop immediately and return a NG report if any step fails — do not proceed to the next step.

The list of files to check is passed in the prompt by the caller. Do not use git diff to discover files.

Always call scripts with relative paths (`./tools/...`). Never use absolute paths for the script itself.

## Steps

### 1. Run format

Pass all `.cpp` and `.hpp` files from the provided file list to `run-format.sh`.

```bash
./tools/run-format.sh <.cpp and .hpp files>
```

Skip if no `.cpp` or `.hpp` files are in the list.

### 2. Run tidy

Pass all `.cpp` files from the provided file list to `run-tidy.sh`.

```bash
./tools/run-tidy.sh <.cpp files>
```

Skip if no `.cpp` files are in the list.

### 3. Build

```bash
./tools/build.sh
```

Terminal output uses `-v:minimal` (errors and warnings only).
If the terminal output is insufficient to diagnose a failure, read `logs/build.log` for details.

### 4. Run tests

```bash
./tools/run-tests.sh
```

The script exits with code 0 on pass, non-zero on failure. Determine pass/fail from the exit code.

## Output

**OK:** All steps passed. Output one line: `CI: OK`

**NG:** A step failed. Output a NG report:

```
CI: NG

## Failed step
<step name>

## Output
<relevant error output — trimmed to essential lines>
```
