---
name: create-pr
description: Create a GitHub PR and automatically run code review
---

Create a GitHub pull request and run a code review on it.

## 手順

### 1. 事前確認

現在のブランチを確認する。

```bash
git branch --show-current
```

main ブランチの場合は作業を中止し、feature/fix ブランチに切り替えるよう伝える。

### 2. PR情報の決定

`$ARGUMENTS` が指定されていればタイトルとして使う。
指定がない場合は `git log main..HEAD --oneline` と `git diff main...HEAD --stat` を参照して、
適切なタイトルを英語・命令形で提案しユーザーに確認する。

本文（body）は `git log main..HEAD --oneline` を元に変更概要を簡潔にまとめる。

### 3. PRの作成

```bash
gh pr create --title "<title>" --body "<body>" --base main
```

作成後、PR の URL を表示する。

### 4. コードレビューの実行

PR 作成直後に、`code-review` プラグインのスキルを使って自動レビューを実行する。
Skill ツールで `code-review:code-review` を呼び出す（引数は PR 番号）。

レビュー完了後、GitHub の PR ページにコメントが投稿されたことをユーザーに伝える。
