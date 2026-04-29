#pragma once

/// @brief フレームごとのプレイヤー入力状態（Siv3D 非依存）
struct InputState {
  /// 左移動キーが押されている
  bool moveLeft = false;
  /// 右移動キーが押されている
  bool moveRight = false;
  /// 奥移動キーが押されている
  bool moveForward = false;
  /// 手前移動キーが押されている
  bool moveBackward = false;
  /// ジャンプキーが押された（1フレーム限り）
  bool jumpDown = false;
  /// 設定再読み込みキーが押された（1フレーム限り）
  bool reloadConfig = false;
};
