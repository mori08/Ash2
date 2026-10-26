#pragma once
#include <Siv3D.hpp>

/// @brief 入力層の調整値
struct InputConfig {
  /// パッド側からキーボード・マウス側へ切り替える、カーソルの累計移動量（px）
  double deviceSwitchCursorDistance;
  /// 方向入力でスティックを倒したとみなす閾値
  double navStickTilt;
  /// 方向入力でスティックを離したとみなす閾値
  double navStickRelease;

  /// @brief TOML から入力設定を生成する
  /// @note `0 < nav_stick_release < nav_stick_tilt <= 1` と
  /// `device_switch_cursor_distance >= 0` を検証する
  /// @throw Error 必須キーの欠落・型違い・不正な値があるとき
  [[nodiscard]] static InputConfig FromToml(const TOMLValue& toml);
};
