---
paths:
  - "Ash2/src/**/*.cpp"
  - "Ash2/src/**/*.hpp"
  - "Ash2/tests/**/*.cpp"
---

# コーディングスタイル

## ファイルの追加

新しい `.cpp` / `.hpp` ファイルを追加するときは、**ビルド前に** `Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集も必要。


## 名前空間

`stdafx.h` の `NO_S3D_USING` は**無効のまま**維持する（`using namespace s3d;` を全体に適用）。

s3d の型・関数は `s3d::` を省略して書く（`Vec2`、`Circle`、`Max` など）。

`std` と `s3d` の両方に同等の関数がある場合は **s3d 側を優先**する（例: `std::max` → `Max`）。

## コメント

`.cpp` / `.hpp` 共通のルール。

### Doxygen コメント

関数・構造体・メンバ変数には `///` で Doxygen コメントを書く。言語は日本語。
**名前・型・シグネチャから読み取れない情報**のみ記述する。設計の経緯・実装の動機は書かない。

- 関数: `@brief` / `@param` / `@return`（いずれも非自明な場合のみ）
- 構造体: `@brief` で一行説明
- メンバ変数: `///` で一行説明（前置）

```cpp
/// @brief ワールド座標
struct WorldPos {
  /// 横位置
  double w = 0.0;
  /// 高さ（地面 = 0、上方向が正）
  double h = 0.0;

  /// @brief ワールド座標を画面座標に変換する
  /// @return 画面座標（右方向・下方向が正）
  [[nodiscard]] Vec2 toScreen() const;
};

/// @brief 描画順の比較（奥→手前、std::sort 用）
/// @return a が b より描画順で前にある場合 true
inline bool DrawOrderLess(const WorldPos& a, const WorldPos& b);
```

### Why not コメント

自然に書いたら別の実装になる箇所で、その理由がコードから読み取れない場合のみ `//` で記述する。

## ARCHITECTURE.md の更新

`docs/ARCHITECTURE.md` はプロジェクトの構造をまとめたドキュメント。

以下の変更を行った場合、実装後・返答前に必ず ARCHITECTURE.md を読み、
現状と乖離があれば更新すること。

- 新しいクラス・構造体の追加
- ディレクトリ構成の変更
- アーキテクチャ方針の変更（フェーズ管理・ECS の使い方など）
- 座標系・ゲームループの変更

### ファイルへのリンク

クラス・システム・フェーズなどを一覧するテーブルでは、名前をリポジトリルートからの相対パスリンクにすること。
リンク形式は `[ClassName](../Ash2/src/Path/To/File.hpp)` （`docs/` からの相対パス）。

### 200行上限のルール

超過しそうな場合は以下の優先順位で削る：
1. コード例（コードを見ればわかる）
2. 実装の詳細（How より Why を残す）
3. 現在使われていない設計の説明

削ってはいけないもの：レイヤー構成・座標系・主要な制約・設計の意図
