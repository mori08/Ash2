---
name: create-issue
description: Create a GitHub issue with a label
---

`gh` CLI を使って GitHub Issue を作成する。

## 引数フォーマット

`<ラベル> <タイトル>` または `<タイトル>`（ラベルは省略可）

## ルール

- 使用可能なラベル: `bug` / `enhancement` / `chore` / `refactor`
- ラベルが引数の先頭に指定されていればそれを使う。なければタイトル・文脈から適切なラベルを推定する
- タイトルは指定されていればそのまま使う。指定がなければユーザーに確認する。**日本語で書く**
- アサインなし
- 本文は追加の文脈がある場合のみ記載する。**日本語で書く**

## 実行コマンド

```
gh issue create --title "<タイトル>" --label "<ラベル>" --body ""
```

作成後、Issue の URL をユーザーに伝える。

## マイルストーン

Issue 作成後、マイルストーンへの登録を行う。

1. `gh api repos/mori08/Ash2/milestones --jq '.[] | "\(.number) \(.title)"'` で一覧を取得してユーザーに提示する
2. ユーザーが選択したマイルストーンを `gh issue edit <number> --milestone "<タイトル>"` で登録する
3. 「登録しない」と言われた場合はスキップする
