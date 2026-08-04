#pragma once
#include <Siv3D.hpp>

#include "Input/InputState.hpp"
#include "Input/KeyboardInputAction.hpp"
#include "Input/XInputAction.hpp"

/// @brief アクティブな入力デバイスを自動検出し InputState を返す
struct InputDeviceSelector {
  /// @brief 毎フレーム呼び出す。最後に入力があったデバイスの状態を返す
  [[nodiscard]] InputState update();

 private:
  enum class Device : uint8 { Keyboard, Gamepad };

  /// キーボード/マウス入力アクション
  KeyboardInputAction m_keyboardAction = KeyboardInputAction::Default();
  Device m_activeDevice = Device::Keyboard;
};

inline InputState InputDeviceSelector::update() {
  // 切断時は即座にキーボードへフォールバック
  if (m_activeDevice == Device::Gamepad && !XInput(0).isConnected()) {
    m_activeDevice = Device::Keyboard;
  }
  // 最後に入力があったデバイスへ切り替える
  const Vec2 stickAxis = XInputAction::KLeftThumbDeadZone(
      Vec2{XInput(0).leftThumbX, XInput(0).leftThumbY}
  );
  if (XInput(0).isConnected() &&
      (XInput(0).buttonUp.down() || XInput(0).buttonDown.down() ||
       XInput(0).buttonLeft.down() || XInput(0).buttonRight.down() ||
       XInput(0).buttonA.down() || XInput(0).buttonB.down() ||
       XInput(0).buttonX.down() || XInput(0).buttonY.down() ||
       !stickAxis.isZero())) {
    m_activeDevice = Device::Gamepad;
  } else if (!Keyboard::GetAllInputs().isEmpty() ||
             !Mouse::GetAllInputs().isEmpty()) {
    m_activeDevice = Device::Keyboard;
  }

  return (m_activeDevice == Device::Gamepad) ? XInputAction::ToInputState()
                                             : m_keyboardAction.toInputState();
}
