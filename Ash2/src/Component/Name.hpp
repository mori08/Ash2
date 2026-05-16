#pragma once
#include <Siv3D.hpp>

/// @brief エンティティ名コンポーネント
struct Name {
  /// エンティティを識別する名前（不変）
  const s3d::String value;
};
