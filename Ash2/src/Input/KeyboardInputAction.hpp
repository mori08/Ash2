#pragma once
#include <Siv3D.hpp>

#include "Input/InputState.hpp"

/// @brief キーボード・マウス操作のキー割り当て
struct KeyboardInputAction {
  InputGroup moveLeft;
  InputGroup moveRight;
  InputGroup moveForward;
  InputGroup moveBackward;
  InputGroup jump;
  InputGroup reloadConfig;
  InputGroup attack;
  InputGroup rangedAttack;
  InputGroup dash;

  /// @brief デフォルトのキー割り当てを返す
  [[nodiscard]] static KeyboardInputAction Default();

  /// @brief 現在のキー入力状態を InputState に変換して返す
  [[nodiscard]] InputState toInputState() const;
};

inline KeyboardInputAction KeyboardInputAction::Default() {
  return {
      .moveLeft = KeyLeft | KeyA,
      .moveRight = KeyRight | KeyD,
      .moveForward = KeyUp | KeyW,
      .moveBackward = KeyDown | KeyS,
      .jump = KeySpace,
      .reloadConfig = KeyF5,
      .attack = MouseL,
      .rangedAttack = MouseR,
      .dash = KeyShift,
  };
}

inline InputState KeyboardInputAction::toInputState() const {
  const Vec2 rawAxis{
      (moveRight.pressed() ? 1.0 : 0.0) - (moveLeft.pressed() ? 1.0 : 0.0),
      (moveForward.pressed() ? 1.0 : 0.0) -
          (moveBackward.pressed() ? 1.0 : 0.0),
  };
  const Vec2 moveAxis = rawAxis.limitLength(1.0);

  return {
      .moveAxis = moveAxis,
      .jumpDown = jump.down(),
      .reloadConfig = reloadConfig.down(),
      .attackDown = attack.down(),
      .rangedAttackDown = rangedAttack.down(),
      .dashDown = dash.down(),
  };
}
