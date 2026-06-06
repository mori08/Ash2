#pragma once
#include <Siv3D.hpp>

#include "Input/InputState.hpp"

/// @brief XInput コントローラー（プレイヤー0）のボタン割り当て
/// @note メンバ変数を持たない。ToInputState()
/// は毎フレームコントローラー状態を直接参照する
struct XInputAction {
  /// @brief 現在のコントローラー入力状態を InputState に変換して返す
  /// @return フレームの入力状態
  [[nodiscard]] static InputState ToInputState();
};

inline InputState XInputAction::ToInputState() {
  const auto& pad = XInput(0);
  return {
      .moveLeft = pad.buttonLeft.pressed(),
      .moveRight = pad.buttonRight.pressed(),
      .moveForward = pad.buttonUp.pressed(),
      .moveBackward = pad.buttonDown.pressed(),
      .jumpDown = pad.buttonA.down(),
      .reloadConfig = false,
      .attackDown = pad.buttonB.down(),
      .rangedAttackDown = pad.buttonY.down(),
  };
}
