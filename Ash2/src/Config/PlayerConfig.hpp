#pragma once
#include <Siv3D.hpp>

/// @brief プレイヤーの設定値
struct PlayerConfig {
  /// 横移動速度（ピクセル/秒）
  double speed;
  /// ジャンプ初速（ピクセル/秒）
  double jumpSpeed;
  /// 重力加速度（ピクセル/秒^2）
  double gravity;

  /// @brief TOML からプレイヤー設定を生成する
  /// @param toml TOML 値
  /// @return PlayerConfig
  [[nodiscard]] static PlayerConfig FromToml(const s3d::TOMLValue& toml);
};
