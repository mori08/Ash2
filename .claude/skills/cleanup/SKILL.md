---
name: cleanup
description: PRマージ後にブランチを整理し、一時ファイルを削除する（resolve-issue Phase 5）
---

# cleanup

PRのマージを確認した上で、ブランチの整理と一時ファイルの削除を行う。

## 手順

### 1. PRのマージ確認

`$ARGUMENTS` から Issue 番号を取得し、対応するPRがマージ済みかを確認する。

```bash
gh pr list --state merged --search "<number>" --json number,title,mergedAt
```

マージされていない場合は作業を中止し、ユーザーに報告する。

### 2. ブランチの整理

以下を順番に実行する。

```bash
git checkout main
git pull origin main
git branch -d <branch-name>
git push origin --delete <branch-name>
```

`git branch -d` が失敗する場合（未マージ判定）は、PRのマージ状態を再確認してからユーザーに相談する。

### 3. 一時ファイルの削除

`tmp/plan-<number>.md` を削除する。

```bash
rm tmp/plan-<number>.md
```

### 4. 完了報告

クローズされた Issue 番号と PR URL をユーザーに伝える。
