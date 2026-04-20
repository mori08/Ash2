#pragma once
#include <Siv3D.hpp>

#include "Input/InputState.hpp"

/// @brief プレイヤー操作のキー割り当て
struct PlayerInputAction {
  /// 左移動
  InputGroup moveLeft;
  /// 右移動
  InputGroup moveRight;
  /// 奥へ移動
  InputGroup moveForward;
  /// 手前へ移動
  InputGroup moveBackward;
  /// ジャンプ
  InputGroup jump;

  /// @brief デフォルトのキー割り当てを返す
  /// @return デフォルトの PlayerInputAction
  [[nodiscard]] static PlayerInputAction Default();

  /// @brief 現在のキー入力状態を InputState に変換して返す
  /// @return フレームの入力状態
  [[nodiscard]] InputState toInputState() const;
};

inline PlayerInputAction PlayerInputAction::Default() {
  return {
      .moveLeft = KeyLeft | KeyA,
      .moveRight = KeyRight | KeyD,
      .moveForward = KeyUp | KeyW,
      .moveBackward = KeyDown | KeyS,
      .jump = KeySpace,
  };
}

inline InputState PlayerInputAction::toInputState() const {
  return {
      .moveLeft = moveLeft.pressed(),
      .moveRight = moveRight.pressed(),
      .moveForward = moveForward.pressed(),
      .moveBackward = moveBackward.pressed(),
      .jumpDown = jump.down(),
  };
}
