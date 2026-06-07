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

  // 左スティックの軸入力にデッドゾーン処理を適用する
  // （`XInput(0)` は const 参照のため `setLeftThumbDeadZone()` を直接呼べず、
  // ここで明示的にデッドゾーンを適用する）
  constexpr DeadZone LeftThumbDeadZone{
      .size = 0.24, .maxValue = 1.0, .type = DeadZoneType::Circular};
  const Vec2 stickAxis =
      LeftThumbDeadZone(Vec2{pad.leftThumbX, pad.leftThumbY});

  // 十字ボタンの入力を軸ベクトルに変換する
  const Vec2 dpadAxis{
      (pad.buttonRight.pressed() ? 1.0 : 0.0) -
          (pad.buttonLeft.pressed() ? 1.0 : 0.0),
      (pad.buttonUp.pressed() ? 1.0 : 0.0) -
          (pad.buttonDown.pressed() ? 1.0 : 0.0),
  };

  // スティックと十字ボタンの両方を同時に有効として扱い、合成してから正規化する
  const Vec2 moveAxis = (stickAxis + dpadAxis).limitLength(1.0);

  return {
      .moveAxis = moveAxis,
      .jumpDown = pad.buttonA.down(),
      .reloadConfig = false,
      .attackDown = pad.buttonB.down(),
      .rangedAttackDown = pad.buttonY.down(),
  };
}
