#pragma once
#include <Siv3D.hpp>

/// @brief エンティティ名コンポーネント
struct Name {
  /// エンティティを識別する名前（不変）
  const s3d::String value;

  /// @brief コンストラクタ
  /// @param v エンティティ名
  explicit Name(s3d::String v) : value(std::move(v)) {}

  Name(const Name&) = default;
};
