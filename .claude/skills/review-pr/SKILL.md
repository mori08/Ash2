---
name: review-pr
description: Fetch a PR diff and perform a code review
---

## 手順

### 1. PR番号の特定

`$ARGUMENTS` が指定されていればその番号を使う。
指定がない場合は `gh pr view --json number -q .number` で現在ブランチのPR番号を取得する。
PRが見つからなければユーザーに番号を尋ねる。

### 2. PRの情報取得

以下を並列で取得する。

```bash
gh pr view <number> --json title,body,author,baseRefName,headRefName
gh pr diff <number>
```

### 3. レビューの実施

取得した diff を元に、以下の観点でレビューを行う。

#### このプロジェクト固有のルール（`.claude/rules/cpp.md`）

- **命名規則**: 静的メンバ関数・free function は PascalCase（大文字始まり）
- **マジックナンバー**: `0`, `1`, `-1`, `2`, `0.0`, `1.0`, `0.5` 以外のリテラルは `constexpr` 定数にする
- **Doxygenコメント**: `.hpp` の関数・構造体・メンバ変数に `///` コメントがあるか
- **ファイル追加時**: `Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集が含まれているか

#### 一般的なC++品質チェック

- メモリ管理（生ポインタの不適切な使用、リーク）
- const 正確性（変更しない引数・メンバ関数への `const` 付与）
- `[[nodiscard]]` の付け忘れ（値を捨てると困る関数）
- 不要なコピー（`const&` や `std::move` の使い忘れ）
- エラーや警告につながりやすいパターン

### 4. 結果の出力

以下の形式でターミナルに出力する。

```
## PR #<number>: <title>

### 必須修正
- （あれば列挙。なければ「なし」）

### 提案
- （あれば列挙。なければ「なし」）

### 良い点
- （あれば列挙）
```

問題が見つからなかった場合は「問題なし」と明記する。
GitHubにコメントは投稿しない。
