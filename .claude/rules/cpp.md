---
paths:
  - "Ash2/src/**/*.cpp"
  - "Ash2/src/**/*.hpp"
  - "Ash2/tests/**/*.cpp"
---

# コーディングスタイル

各節の詳細は docs 側に切り出してある。**節に書かれたタイミングになったら、必ずリンク先を読むこと。**

## ファイルの追加

新しい `.cpp` / `.hpp` ファイルを追加するときは、**ビルド前に** `Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集も必要。

## s3d の優先使用

`stdafx.h` の `NO_S3D_USING` は**無効のまま**維持する（`using namespace s3d;` を全体に適用）。

s3d の型・関数は `s3d::` を省略して書く（`Vec2`、`Circle`、`Max` など）。

`std` と `s3d` の両方に同等の型・関数がある場合は **s3d 側を優先**する（例: `std::max` → `Max`、`std::optional` → `Optional`）。
s3d に相当がないものは `std` をそのまま使う（例: `std::expected`）。

## エラー処理

| 状況 | 手段 |
|------|------|
| プログラマのバグ（不変条件・契約違反） | `assert(条件 && "日本語メッセージ")` |
| 「値がない」ことが正常系の一部 | `Optional<T>`（関数は `[[nodiscard]]`） |
| それ以外の失敗（第一候補） | `std::expected<T, E>`（関数は `[[nodiscard]]`） |
| 初期化経路の最上位層で失敗を致命として確定させる（最後の手段） | `throw Error{...}` |

例外はゲームループ内（毎フレーム実行される `Phase::update` / System）では投げない。
新たに throw を書くときは、`Optional` / `expected` で表現しない理由を Why not コメントで添える。

使い分けの判断手順と詳細は [docs/ERROR_HANDLING.md](../../docs/ERROR_HANDLING.md) にある。
**エラーを扱うコードを書く・レビューする前に必ず読むこと。**

## コメント

- 関数・構造体・メンバ変数には `///` の Doxygen コメント（日本語）。
  名前・型・シグネチャから読み取れない情報のみ書く
- Why not コメント：自然に書いたら別の実装になる箇所で、
  その理由がコードから読み取れない場合のみ `//` で書く

記法と例は [docs/COMMENT.md](../../docs/COMMENT.md) にある。
**新しい関数・構造体・メンバ変数を書く前に必ず読むこと。**

## ドキュメントの更新

ディレクトリ構成・アーキテクチャ方針・座標系・ゲームループを変更したら `docs/ARCHITECTURE.md` を、
コンポーネント・システム・フェーズ等を追加・削除・仕様変更したら `docs/REFERENCE.md` を更新する。

更新の判断基準と記述ルールは [docs/DOCUMENTATION.md](../../docs/DOCUMENTATION.md) にある。
**上記に該当する実装を終えたら、返答前に必ず読むこと。**
