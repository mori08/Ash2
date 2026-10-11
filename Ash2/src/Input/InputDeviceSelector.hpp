#pragma once
#include <Siv3D.hpp>

#include "Config/InputConfig.hpp"
#include "Input/InputState.hpp"
#include "Input/KeyboardInputAction.hpp"
#include "Input/XInputAction.hpp"

/// @brief アクティブな入力デバイスを自動検出し InputState を返す
struct InputDeviceSelector {
  enum class Device : uint8 { Keyboard, Gamepad };

  /// そのフレームに観測したデバイスの入力
  struct DeviceSignals {
    bool padConnected;
    /// ボタンの down、またはスティックの傾き
    bool padInput;
    /// Keyboard / Mouse の `GetAllInputs()` が空でない
    bool keyboardMouseButton;
    /// `Cursor::DeltaRaw()` の長さ（クライアント座標の px）
    double cursorTravel;
  };

  /// 選んだデバイスと、パッド側にいる間のカーソル移動量の積算
  struct DeviceChoice {
    Device device;
    double padCursorTravel;
  };

  /// @brief 今フレームのアクティブデバイスを決める
  /// @param prev 前フレームの選択
  /// @param switchDistance パッド側からキーボード・マウス側へ切り替える
  /// 累計移動量。これを超えたら切り替える
  /// @note パッド入力とカーソル移動が同じフレームならパッドを優先する。
  /// 積算はパッド側にいる間だけ行う
  [[nodiscard]] static DeviceChoice ChooseDevice(
      const DeviceChoice& prev, const DeviceSignals& signals,
      double switchDistance
  );

  /// @brief 毎フレーム呼び出す。最後に入力があったデバイスの状態を返す
  /// @note 非アクティブなデバイスも毎フレーム評価する
  [[nodiscard]] InputState update(const InputConfig& config);

 private:
  /// キーボード/マウス入力アクション
  KeyboardInputAction m_keyboardAction = KeyboardInputAction::Default();
  /// ゲームパッド入力アクション
  XInputAction m_gamepadAction;
  Device m_activeDevice = Device::Keyboard;
  /// パッド側にいる間のカーソル移動量の積算
  double m_padCursorTravel = 0.0;
};

inline InputDeviceSelector::DeviceChoice InputDeviceSelector::ChooseDevice(
    const DeviceChoice& prev, const DeviceSignals& signals,
    double switchDistance
) {
  // 切断時は即座にキーボードへフォールバック
  if (prev.device == Device::Gamepad && !signals.padConnected) {
    return {.device = Device::Keyboard, .padCursorTravel = 0.0};
  }
  if (signals.padConnected && signals.padInput) {
    return {.device = Device::Gamepad, .padCursorTravel = 0.0};
  }
  if (signals.keyboardMouseButton) {
    return {.device = Device::Keyboard, .padCursorTravel = 0.0};
  }
  if (prev.device == Device::Gamepad) {
    // フレーム単位ではゆっくりした移動が閾値に届かないため積算する
    const double travel = prev.padCursorTravel + signals.cursorTravel;
    if (travel > switchDistance) {
      return {.device = Device::Keyboard, .padCursorTravel = 0.0};
    }
    return {.device = Device::Gamepad, .padCursorTravel = travel};
  }
  return prev;
}

inline InputState InputDeviceSelector::update(const InputConfig& config) {
  const auto& pad = XInput(0);
  // 右スティックだけを動かした場合もパッドへ切り替える。ゲームプレイ上の
  // 傾き判定（LockConfig）とは目的が違うため閾値は共有せず、ここでは
  // デッドゾーンのみ通す（生値のまま isZero() を見るとスティックの
  // ドリフトで毎フレーム Device::Gamepad に固定されてしまう）
  const Vec2 leftStickAxis =
      XInputAction::kLeftThumbDeadZone(Vec2{pad.leftThumbX, pad.leftThumbY});
  const Vec2 rightStickAxis =
      XInputAction::kLeftThumbDeadZone(Vec2{pad.rightThumbX, pad.rightThumbY});

  const DeviceSignals signals{
      .padConnected = pad.isConnected(),
      .padInput =
          (pad.buttonUp.down() || pad.buttonDown.down() ||
           pad.buttonLeft.down() || pad.buttonRight.down() ||
           pad.buttonA.down() || pad.buttonB.down() || pad.buttonX.down() ||
           pad.buttonY.down() || !leftStickAxis.isZero() ||
           !rightStickAxis.isZero()),
      .keyboardMouseButton =
          (!Keyboard::GetAllInputs().isEmpty() ||
           !Mouse::GetAllInputs().isEmpty()),
      .cursorTravel = Vec2{Cursor::DeltaRaw()}.length(),
  };
  const DeviceChoice choice = ChooseDevice(
      {.device = m_activeDevice, .padCursorTravel = m_padCursorTravel}, signals,
      config.deviceSwitchCursorDistance
  );
  m_activeDevice = choice.device;
  m_padCursorTravel = choice.padCursorTravel;

  // 両デバイスを評価してから片方を返す。パッドが非アクティブでも
  // スティックの前フレーム状態を更新し続けないと、切り替え直後に古い値で
  // 方向入力を取りこぼす
  const InputState keyboardState = m_keyboardAction.toInputState();
  const InputState gamepadState = m_gamepadAction.toInputState(config);
  return (m_activeDevice == Device::Gamepad) ? gamepadState : keyboardState;
}
