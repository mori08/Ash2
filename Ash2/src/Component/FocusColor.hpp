#pragma once
#include <Siv3D.hpp>

/// @brief 選択状態に応じて切り替える描画色
///
/// `FocusColorSystem` が `Focused` の有無で `DrawColor` へ書く。
struct FocusColor {
  /// 選択されていないときの色
  ColorF normal{1.0};
  /// 選択中の色
  ColorF focused{1.0};
};
