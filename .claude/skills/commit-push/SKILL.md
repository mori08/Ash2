---
name: commit-push
description: ファイルをステージし、適切なメッセージでコミットしてリモートにプッシュする
---

最初に `docs/GIT.md` を読み、`git branch` で現在のブランチを確認する（並列実行）。

## main ブランチの場合

作業が `chore`（直接コミット可）か `enhancement`/`bug`（feature/fix ブランチ＋PR が必要）かを確認する。
`chore` でない場合は作業を中止し、先にブランチを作成するよう伝える。

## feature/fix ブランチの場合

`git log main..HEAD --oneline` で main からのコミット数を確認する。
複数コミットがある場合は GIT.md のコミット整理ルールを適用し、まとめるか分けるかの判断をユーザーに伝えてから進む。

---

`git status` と `git diff` を並列実行して変更内容を把握する。

以下のルールに従ってステージ・コミット・プッシュを行う。

- コミットメッセージ: 英語・命令形・プレフィックスなし（例: "Add X"、"Fix Y"、"Update Z"）
- issue 参照: 関連 issue がある場合は必ず含める（例: `#3` で参照、`close #3` でマージ時に自動クローズ）。issue を立てずに行う `chore` コミットは省略可
- コミット直後に必ずプッシュする
- ステージは `git add -A` でなくファイル名を指定する
- コミットメッセージ末尾に付記する: `Co-Authored-By: Claude <実行中のモデル名> <noreply@anthropic.com>`（モデル名は固定文字列にせず、その時点で自分が実行しているモデルの名称を使う。例: Claude Sonnet 5）

`$ARGUMENTS` でメッセージが指定された場合はそのまま使う。指定がない場合は diff からメッセージを作成する。
