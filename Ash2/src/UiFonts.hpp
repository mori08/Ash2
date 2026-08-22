#pragma once
#include <Siv3D.hpp>

#include <expected>

/// @brief UI 描画に使うフォント一式（registry.ctx() に格納）
struct UiFonts {
  Font large;
  Font small;

  /// @return 生成に失敗した場合はサイズを含むメッセージ
  [[nodiscard]] static std::expected<UiFonts, String> Create() {
    constexpr int32 kLargeSize = 24;
    constexpr int32 kSmallSize = 20;
    UiFonts fonts{.large = Font{kLargeSize}, .small = Font{kSmallSize}};
    if (fonts.large.isEmpty()) {
      return std::unexpected{
          U"UiFonts::Create: サイズ {} のフォントを生成できません"_fmt(
              kLargeSize
          )
      };
    }
    if (fonts.small.isEmpty()) {
      return std::unexpected{
          U"UiFonts::Create: サイズ {} のフォントを生成できません"_fmt(
              kSmallSize
          )
      };
    }
    return fonts;
  }
};
