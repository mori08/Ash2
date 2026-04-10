#pragma once
#include <Siv3D.hpp>

/// @brief プレイヤー操作のキー割り当て
struct PlayerInputAction {
  /// 左移動
  InputGroup moveLeft;
  /// 右移動
  InputGroup moveRight;

  /// @brief デフォルトのキー割り当てを返す
  /// @return デフォルトの PlayerInputAction
  [[nodiscard]] static PlayerInputAction Default();
};

inline PlayerInputAction PlayerInputAction::Default() {
  return {
      .moveLeft = KeyLeft | KeyA,
      .moveRight = KeyRight | KeyD,
  };
}
