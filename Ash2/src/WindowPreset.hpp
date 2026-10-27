#pragma once
#include <Siv3D.hpp>

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
/// @throw Error ウィンドウの大きさを変更できないとき
inline void ApplyWindowPreset(WindowPreset preset) {
  switch (preset) {
    case WindowPreset::Size1280x720: {
      constexpr Size kSize{1280, 720};
      if (!Window::Resize(kSize)) {
        throw Error{U"ウィンドウの大きさを変更できない"};
      }
      return;
    }
  }
  std::unreachable();
}
