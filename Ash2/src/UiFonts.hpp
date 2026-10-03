#pragma once
#include <Siv3D.hpp>

#include <array>
#include <expected>

namespace UiFonts {

/// 見出し用（FontAsset のキー）
inline constexpr AssetNameView kLarge = U"ui/large";
/// 本文用（FontAsset のキー）
inline constexpr AssetNameView kSmall = U"ui/small";

/// @brief UI フォントを FontAsset に登録し、読み込みまで済ませる
/// @return 失敗時はキーとサイズを含むメッセージ
[[nodiscard]] inline std::expected<void, String> Register() {
  struct Entry {
    AssetNameView key;
    int32 size;
  };
  constexpr std::array kEntries{
      Entry{.key = kLarge, .size = 24}, Entry{.key = kSmall, .size = 20}
  };
  for (const auto& [key, size] : kEntries) {
    if (!FontAsset::Register(key, size)) {
      return std::unexpected{
          U"UiFonts::Register: {} を登録できません"_fmt(key)
      };
    }
    if (!FontAsset::Load(key)) {
      return std::unexpected{
          U"UiFonts::Register: {}（サイズ {}）を読み込めません"_fmt(key, size)
      };
    }
  }
  return {};
}

}  // namespace UiFonts
