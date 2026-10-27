#pragma once
#include <Siv3D.hpp>

#include <array>

namespace UiFonts {

/// 見出し用（FontAsset のキー）
inline constexpr AssetNameView kLarge = U"ui/large";
/// 本文用（FontAsset のキー）
inline constexpr AssetNameView kSmall = U"ui/small";

/// @brief UI フォントを FontAsset に登録し、読み込みまで済ませる
/// @throw Error 登録または読み込みに失敗したとき
inline void Register() {
  struct Entry {
    AssetNameView key;
    int32 size;
  };
  constexpr std::array kEntries{
      Entry{.key = kLarge, .size = 24}, Entry{.key = kSmall, .size = 20}
  };
  for (const auto& [key, size] : kEntries) {
    if (!FontAsset::Register(key, size)) {
      throw Error{U"UI フォントをアセットに登録できない"};
    }
    if (!FontAsset::Load(key)) {
      throw Error{U"UI フォントを読み込めない"};
    }
  }
}

}  // namespace UiFonts
