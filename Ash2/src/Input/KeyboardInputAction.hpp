#pragma once
#include <Siv3D.hpp>

#include "Input/InputState.hpp"

/// @brief キーボード・マウス操作のキー割り当て
struct KeyboardInputAction {
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
  /// 設定再読み込み
  InputGroup reloadConfig;
  /// 近距離攻撃
  InputGroup attack;
  /// 遠距離攻撃
  InputGroup rangedAttack;

  /// @brief デフォルトのキー割り当てを返す
  /// @return デフォルトの KeyboardInputAction
  [[nodiscard]] static KeyboardInputAction Default();

  /// @brief 現在のキー入力状態を InputState に変換して返す
  /// @return フレームの入力状態
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
  };
}
