---
paths:
  - "Ash2/src/**/*.cpp"
  - "Ash2/src/**/*.hpp"
  - "Ash2/tests/**/*.cpp"
---

# コーディングスタイル

## フォーマット自動適用

clang-format はバージョン **19 系**を使用する。

`.cpp` / `.hpp` ファイルを編集したら、必ず以下のコマンドでフォーマットをかける：

```bash
clang-format -i <ファイルパス>
# PATH に無い場合: find "/c/Program Files/Microsoft Visual Studio" -name "clang-format.exe"
```

## コメント（.hpp）

`.hpp` の関数・構造体・メンバ変数には `///` で Doxygen コメントを書く。言語は日本語。

- 関数: `@brief`（必須）、`@param` / `@return`（引数・戻り値がある場合は必須）
- 構造体: `@brief` で一行説明
- メンバ変数: `///` で一行説明（前置）
- 自明なもの（getter 等）は省略可

```cpp
/// @brief ワールド座標
struct WorldPos {
  /// 横位置
  double w = 0.0;
  /// 高さ（地面 = 0、上方向が正）
  double h = 0.0;

  /// @brief ワールド座標を画面座標に変換する
  /// @return 画面座標（右方向・下方向が正）
  [[nodiscard]] Vec2 ToScreen() const;
};

/// @brief 描画順の比較関数（奥から手前の順）
/// @param a 比較対象A
/// @param b 比較対象B
/// @return a が b より奥にある場合 true
inline bool DrawOrderLess(const WorldPos& a, const WorldPos& b);
```

## ファイルの追加

新しい `.cpp` / `.hpp` ファイルを追加するときは、`Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集も必要。

## ARCHITECTURE.md の更新

`docs/ARCHITECTURE.md` はプロジェクトの構造をまとめたドキュメント。

以下の変更を行った場合、実装後・返答前に必ず ARCHITECTURE.md を読み、
現状と乖離があれば更新すること。

- 新しいクラス・構造体の追加
- ディレクトリ構成の変更
- アーキテクチャ方針の変更（フェーズ管理・ECS の使い方など）
- 座標系・ゲームループの変更

### 200行上限のルール

超過しそうな場合は以下の優先順位で削る：
1. コード例（コードを見ればわかる）
2. 実装の詳細（How より Why を残す）
3. 現在使われていない設計の説明

削ってはいけないもの：レイヤー構成・座標系・主要な制約・設計の意図
