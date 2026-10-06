#pragma once
#include <Siv3D.hpp>

#include <expected>
#include <utility>

/// @brief ウィンドウの表示サイズの候補
///
/// 列挙子を足すときは ApplyWindowPreset の switch も更新する。
enum class WindowPreset : uint8 {
  Size1280x720,
};

/// 起動時に適用するプリセット
inline constexpr WindowPreset kDefaultWindowPreset = WindowPreset::Size1280x720;

/// @brief プリセットをウィンドウへ適用する
///
/// 大きさは仮想サイズ基準のクライアント領域の px。
/// @return 失敗時はプリセットの大きさを含むメッセージ
[[nodiscard]] inline std::expected<void, String> ApplyWindowPreset(
    WindowPreset preset
) {
  switch (preset) {
    case WindowPreset::Size1280x720: {
      constexpr Size kSize{1280, 720};
      if (!Window::Resize(kSize)) {
        return std::unexpected{
            U"ApplyWindowPreset: {}x{} に変更できません"_fmt(kSize.x, kSize.y)
        };
      }
      return {};
    }
  }
  std::unreachable();
}
