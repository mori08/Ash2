---
name: create-issue
description: Create a GitHub issue with a label
---

`gh` CLI を使って GitHub Issue を作成する。

## 引数フォーマット

`<ラベル> <タイトル>` または `<タイトル>`（ラベルは省略可）

## ルール

- 使用可能なラベル: `bug` / `enhancement` / `chore` / `refactor` / `epic`
- ラベルが引数の先頭に指定されていればそれを使う。なければタイトル・文脈から適切なラベルを推定する
- `epic` は引数で明示指定されたときのみ使う
- タイトルは指定されていればそのまま使う。指定がなければユーザーに確認する。日本語で記載する
- アサインなし
- 本文は追加の文脈がある場合のみ記載する。日本語で記載する

## 実行コマンド

```
gh issue create --title "<タイトル>" --label "<ラベル>" --body ""
```

作成後、Issue の URL をユーザーに伝える。

## マイルストーン

Issue 作成後、マイルストーンへの登録を行う。

1. `docs/ROADMAP.md` を読み、Issue の内容（タイトル・ラベル・文脈）からどのバージョンのスコープに該当するかを判断して、マイルストーンを1つ提案する
2. `gh api -X GET repos/mori08/Ash2/milestones --jq '.[] | "\(.number) \(.title)"'` で一覧を取得し、提案と一緒にユーザーに提示する
3. ユーザーが選択したマイルストーンを `gh issue edit <number> --milestone "<タイトル>"` で登録する
4. 「登録しない」と言われた場合はスキップする
