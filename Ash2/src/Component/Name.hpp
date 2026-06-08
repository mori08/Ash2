#pragma once
#include <Siv3D.hpp>

/// @brief エンティティ名コンポーネント
struct Name {
  // NameLookup は構築・破棄シグナルでのみ同期されるため、構築後に value を
  // 変更すると対応がずれる
  const s3d::String value;

  explicit Name(s3d::String v) : value(std::move(v)) {}

  Name(const Name&) = default;
};
