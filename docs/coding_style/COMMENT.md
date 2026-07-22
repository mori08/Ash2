# COMMENT.md

`.cpp` / `.hpp` 共通のコメント規約。

## Doxygen コメント

関数・構造体・メンバ変数には `///` で Doxygen コメントを書く。言語は日本語。
**名前・型・シグネチャから読み取れない情報**のみ記述する。設計の経緯・実装の動機は書かない。

- 関数: `@brief` / `@param` / `@return`（いずれも非自明な場合のみ）
- 構造体: `@brief` で一行説明
- メンバ変数: `///` で一行説明（前置）

記入例（例示用の架空コード。実コードとは同期しない）：

```cpp
/// @brief 円形の当たり判定
struct HitCircle {
  /// 中心のワールド座標
  Vec2 center{0.0, 0.0};
  /// 半径（ピクセル）
  double radius = 0.0;

  /// @brief 点が判定内にあるか
  /// @return 境界線上を含めて true
  [[nodiscard]] bool contains(Vec2 point) const;
};

/// @brief 半径の大きい順の比較（std::sort 用）
/// @return a が b より半径が大きい場合 true
inline bool RadiusGreater(const HitCircle& a, const HitCircle& b);
```

## Why not コメント

自然に書いたら別の実装になる箇所で、その理由がコードから読み取れない場合のみ `//` で記述する。
