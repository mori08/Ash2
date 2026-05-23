---
name: create-pr
description: Create a GitHub PR
---

Create a GitHub pull request and run a code review on it.

Before doing anything, read `docs/GIT.md`.

## 手順

### 1. 事前確認

以下を並行して実行する。

```bash
git branch --show-current
git log main..HEAD --oneline
```

main ブランチの場合は作業を中止し、feature/fix ブランチに切り替えるよう伝える。

コミットログから対応する Issue 番号を特定し、`gh issue view <number>` で Issue の内容を確認する。
Issue のスコープと実装内容にズレがあれば、ユーザーに報告し、必要に応じて Issue にコメントを追加してから次のステップへ進む。

### 2. PR情報の決定

`$ARGUMENTS` が指定されていればタイトルとして使う。
指定がない場合は `git log main..HEAD --oneline` と `git diff main...HEAD --stat` を参照して、
適切なタイトルを日本語で提案しユーザーに確認する。

本文（body）は `git log main..HEAD --oneline` を元に変更概要を日本語で簡潔にまとめる。

### 3. PRの作成

```bash
gh pr create --title "<title>" --body "<body>" --base main
```

作成後、PR の URL を表示する。

### 4. コードレビューの実行

PR 作成直後に、Agent ツールで `pr-reviewer` エージェントを起動する。
エージェントへの入力に以下を含めること。

- PR 番号
- ローカルリポジトリのパス
- ファイルを読む際は `gh api` を使わず `Read` ツールでローカルファイルを直接参照すること

エージェントが完了すると GitHub の PR ページにレビューコメントが投稿される。
投稿完了後、PR の URL をユーザーに伝える。
