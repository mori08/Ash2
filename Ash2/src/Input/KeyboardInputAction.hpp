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
  InputGroup attack;
  InputGroup rangedAttack;
  InputGroup dash;

  /// @brief デフォルトのキー割り当てを返す
  [[nodiscard]] static KeyboardInputAction Default();

  /// @brief 現在のキー入力状態を InputState に変換して返す
  [[nodiscard]] InputState toInputState() const;

  /// @brief カーソルが動いたかを判定し、動いていれば現在位置を返す
  /// @param deltaRaw `Cursor::DeltaRaw()`（クライアント座標の移動量）
  /// @param pos 現在のカーソル位置
  /// @note 1px でも動けば値を返す
  [[nodiscard]] static Optional<Vec2> PointerMoved(Point deltaRaw, Vec2 pos);
};

inline KeyboardInputAction KeyboardInputAction::Default() {
  return {
      .moveLeft = KeyLeft | KeyA,
      .moveRight = KeyRight | KeyD,
      .moveForward = KeyUp | KeyW,
      .moveBackward = KeyDown | KeyS,
      .jump = KeySpace,
      .attack = MouseL,
      .rangedAttack = MouseR,
      .dash = KeyShift,
  };
}

inline Optional<Vec2> KeyboardInputAction::PointerMoved(
    Point deltaRaw, Vec2 pos
) {
  // ウィンドウの拡縮によるシーン座標のずれを動きとして扱わないため、
  // シーン座標の差分は使わず DeltaRaw で判定する
  if (deltaRaw.isZero()) {
    return none;
  }
  return pos;
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
      .attackDown = attack.down(),
      .rangedAttackDown = rangedAttack.down(),
      .dashDown = dash.down(),
      .pointerPos = Cursor::PosF(),
      .clickDown = MouseL.down(),
      .pointerMoved = PointerMoved(Cursor::DeltaRaw(), Cursor::PosF()),
  };
}
