---
paths:
  - "Ash2/src/**/*.cpp"
  - "Ash2/src/**/*.hpp"
  - "Ash2/tests/**/*.cpp"
review:
  - エラー処理
  - コメント
---

# コーディングスタイル

## ファイルの追加

- 条件：新しい `.cpp` / `.hpp` ファイルを追加するとき

ビルド前に `Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集も必要。

## s3d の優先使用

- 条件：常時（すべての C++ コード）

`stdafx.h` の `NO_S3D_USING` は無効のまま維持する（`using namespace s3d;` を全体に適用）。

s3d の型・関数は `s3d::` を省略して書く（`Vec2`、`Circle`、`Max` など）。

`std` と `s3d` の両方に同等の型・関数がある場合は s3d 側を優先する（例: `std::max` → `Max`、`std::optional` → `Optional`）。
s3d に相当がないものは `std` をそのまま使う（例: `std::expected`）。

組み込みの数値型は以下を使う。

- 整数: `int32`（s3d の固定幅エイリアス）。ただし添字・サイズには `size_t` を使う
- 小数: `double`
- 型幅に意図がある場合（`enum class` の基底型など）は `uint8` 等の固定幅エイリアスを使う

## ECS

- 条件：コンポーネント・システムを追加する、またはビューを書く前
- 必読：[docs/coding_style/ECS.md](../../docs/coding_style/ECS.md)（ビューの書き方と禁止事項）

- 種別で `if` を書かず、コンポーネントの組み合わせでビューを絞る
- 振る舞いを止めるときは `entt::exclude` でビューから外す。`bool` のスイッチを持たせない
- 時間で進むシステムは `Hitstop` を除外するか `dt = 0` で呼ぶかを決める
- システムは静的関数。フレームをまたぐ状態はコンポーネントか `registry.ctx()` に置く

## エラー処理

- 条件：エラーを扱うコードを書く・レビューする前
- 必読：[docs/coding_style/ERROR_HANDLING.md](../../docs/coding_style/ERROR_HANDLING.md)（手段の選び方と終了の決め方）

| 状況 | 手段 |
|------|------|
| コードが正しければ起きない（不変条件・契約違反） | `assert(条件 && "日本語メッセージ")` |
| 「値がない」ことが正常系 | `Optional<T>` |
| それ以外の失敗 | `std::expected<T, E>` |
| 受け取った失敗に対して終了を決める | `throw FatalError{...}` |

`Optional` / `expected` を返す関数には `[[nodiscard]]` を付ける。
`Optional` / `expected` を返せる関数の中では終了を決めない。

## コメント

- 条件：新しい関数・構造体・メンバ変数、または注記コメントを書く前
- 必読：[docs/coding_style/COMMENT.md](../../docs/coding_style/COMMENT.md)（記法と例）

- 関数・構造体・メンバ変数には `///` の Doxygen コメント（日本語）。
  名前・型・シグネチャから読み取れない情報のみ書く
- Why not コメント：自然に書いたら別の実装になる箇所で、
  その理由がコードから読み取れない場合のみ `//` で書く
- TODO コメント：未解決の問題を `// TODO(#<Issue番号>): <症状>` の形式で書く

## ドキュメントの更新

- 条件：以下に該当する実装を終えたとき（返答前）
- 必読：[docs/coding_style/DOCUMENTATION.md](../../docs/coding_style/DOCUMENTATION.md)（更新の判断基準と記述ルール）

ディレクトリ構成・アーキテクチャ方針・座標系・ゲームループを変更したら `docs/ARCHITECTURE.md` を、
コンポーネント・システム・フェーズ等を追加・削除・仕様変更したら `docs/REFERENCE.md` を更新する。
