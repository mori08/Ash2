---
paths:
  - "Ash2/src/**/*.cpp"
  - "Ash2/src/**/*.hpp"
  - "Ash2/tests/**/*.cpp"
---

# コーディングスタイル

フォーマットは `.clang-format`、静的解析は `.clang-tidy` で自動管理。以下は自動化できないルールのみ記載。

## インクルード順序

1. 対応ヘッダ（`Foo.cpp` なら `Foo.hpp`）
2. `<Siv3D.hpp>`
3. その他ローカルヘッダ `"..."`
4. その他システムヘッダ `<...>`

`.clang-format` の `IncludeCategories` で自動ソートされる。

## 命名規則

`.clang-tidy` の `readability-identifier-naming` で自動チェックされる。

| 対象 | スタイル | 例 |
|---|---|---|
| 変数 | camelCase | `count`, `playerPos` |
| 関数 | PascalCase | `Update()`, `GetRect()` |
| クラス・構造体 | PascalCase | `Player`, `GameScene` |
| プライベートメンバ変数 | `m_` + camelCase | `m_hp`, `m_texture` |
| enum class | PascalCase（型・値ともに） | `enum class Direction { Up, Down }` |
| constexpr 定数 | PascalCase | `SceneWidth`, `MaxEnemyCount` |
| ファイル名 | PascalCase | `Player.cpp`, `GameScene.hpp` |

構造体（`struct`）は公開メンバのみの場合に使用し、それ以外は `class` を使用する。

## ファイルの追加

新しい `.cpp` / `.hpp` ファイルを追加するときは、`Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集も必要。

## コードスタイル

| ルール | 良い例 | 悪い例 |
|---|---|---|
| 論理否定は `not` を使う | `if (not isActive)` | `if (!isActive)` |
| プリインクリメントを使う | `++i` | `i++` |
| 浮動小数点は両側に数字を書く | `1.0`, `0.5` | `1.`, `.5` |
| 比較は `<` / `<=` を優先する | `a < b` | `b > a` |
| インクルードガードは `#pragma once` を使う | `#pragma once` | `#ifndef ...` |
