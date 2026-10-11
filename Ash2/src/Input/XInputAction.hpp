#pragma once
#include <Siv3D.hpp>

#include "Config/InputConfig.hpp"
#include "Input/InputState.hpp"

/// @brief XInput コントローラー（プレイヤー0）のボタン割り当て
/// @note パッドが非アクティブなフレームでも毎フレーム toInputState() を
/// 呼ぶこと
struct XInputAction {
  /// 左スティックに適用するデッドゾーン（`InputDeviceSelector`
  /// もスティック傾き判定に同じ定数を参照する）
  static constexpr DeadZone kLeftThumbDeadZone{
      .size = 0.24, .maxValue = 1.0, .type = DeadZoneType::Circular
  };

  /// @brief スティックの傾きから方向を求める
  /// @param threshold この値以上の傾きで方向ありとみなす
  /// @return 傾きの大きい軸の方向。同じ大きさなら縦を優先する。
  /// 閾値未満なら none
  /// @note y は上が正（`NavDirection::Up` に対応する）
  [[nodiscard]] static Optional<NavDirection> StickDirection(
      Vec2 stick, double threshold
  );

  /// @brief ヒステリシス付きで、スティックを倒している方向を求める
  /// @param prev 前フレームに倒していた方向
  /// @note 倒していない間は `tilt`、倒している間は `release` を閾値にする
  [[nodiscard]] static Optional<NavDirection> HeldStickDirection(
      Vec2 stick, Optional<NavDirection> prev, double tilt, double release
  );

  /// @brief 現在のコントローラー入力状態を InputState に変換して返す
  [[nodiscard]] InputState toInputState(const InputConfig& config);

 private:
  /// 前フレームにスティックを倒していた方向
  Optional<NavDirection> m_stickNav = none;
};

inline Optional<NavDirection> XInputAction::StickDirection(
    Vec2 stick, double threshold
) {
  const double absX = Abs(stick.x);
  const double absY = Abs(stick.y);
  if (Max(absX, absY) < threshold) {
    return none;
  }
  if (absY >= absX) {
    return (stick.y > 0.0) ? NavDirection::Up : NavDirection::Down;
  }
  return (stick.x > 0.0) ? NavDirection::Right : NavDirection::Left;
}

inline Optional<NavDirection> XInputAction::HeldStickDirection(
    Vec2 stick, Optional<NavDirection> prev, double tilt, double release
) {
  return StickDirection(stick, prev ? release : tilt);
}

inline InputState XInputAction::toInputState(const InputConfig& config) {
  const auto& pad = XInput(0);

  // `XInput(0)` は const 参照のため `setLeftThumbDeadZone()` を直接呼べず、
  // ここで明示的にデッドゾーンを適用する
  const Vec2 stickAxis =
      kLeftThumbDeadZone(Vec2{pad.leftThumbX, pad.leftThumbY});

  const Vec2 dpadAxis{
      (pad.buttonRight.pressed() ? 1.0 : 0.0) -
          (pad.buttonLeft.pressed() ? 1.0 : 0.0),
      (pad.buttonUp.pressed() ? 1.0 : 0.0) -
          (pad.buttonDown.pressed() ? 1.0 : 0.0),
  };

  // スティックと十字ボタンの両方を同時に有効として扱い、合成してから正規化する
  const Vec2 moveAxis = (stickAxis + dpadAxis).limitLength(1.0);

  // 右スティックは画面座標系（右が正・下が正）へ合わせるため y を反転する。
  // 0.5/0.24 の2段階閾値は LockOnSystem 側で効かせるため、ここではデッド
  // ゾーンを掛けず生値のまま渡す
  const Vec2 lockAxis{pad.rightThumbX, -pad.rightThumbY};

  // スティックの状態は、方向入力を出さないフレームでも更新する
  const Optional<NavDirection> heldStick = HeldStickDirection(
      Vec2{pad.leftThumbX, pad.leftThumbY}, m_stickNav, config.navStickTilt,
      config.navStickRelease
  );
  const bool stickJustTilted = heldStick && heldStick != m_stickNav;
  const Optional<NavDirection> stickPressed =
      stickJustTilted ? heldStick : Optional<NavDirection>{};
  m_stickNav = heldStick;

  // 十字ボタンを優先する。同時に押された場合は上→下→左→右の順で1つ選ぶ
  Optional<NavDirection> navigate = stickPressed;
  if (pad.buttonUp.down()) {
    navigate = NavDirection::Up;
  } else if (pad.buttonDown.down()) {
    navigate = NavDirection::Down;
  } else if (pad.buttonLeft.down()) {
    navigate = NavDirection::Left;
  } else if (pad.buttonRight.down()) {
    navigate = NavDirection::Right;
  }

  return {
      .moveAxis = moveAxis,
      .jumpDown = pad.buttonA.down(),
      .attackDown = pad.buttonB.down(),
      .rangedAttackDown = pad.buttonX.down(),
      .dashDown = pad.buttonY.down(),
      .lockAxis = lockAxis,
      .navigate = navigate,
      .confirmDown = pad.buttonA.down(),
  };
}
