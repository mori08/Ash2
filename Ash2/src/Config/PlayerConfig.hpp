#pragma once
#include <Siv3D.hpp>

#include "Component/WorldPos.hpp"

/// @brief 扇形パーツの設定値
struct PlayerPiePartConfig {
  /// 親からの相対座標
  WorldPos offset;
  /// 半径
  double radius;
  /// 開始角度（ラジアン、12時方向から時計回り）
  double startAngle;
  /// 角度（ラジアン）
  double angle;
  /// 描画色
  s3d::ColorF color;
};

/// @brief 円形パーツの設定値
struct PlayerCirclePartConfig {
  /// 親からの相対座標
  WorldPos offset;
  /// 半径
  double radius;
  /// 描画色
  s3d::ColorF color;
};

/// @brief プレイヤーの設定値
struct PlayerConfig {
  /// 横移動速度（ピクセル/秒）
  double speed;
  /// ジャンプ初速（ピクセル/秒）
  double jumpSpeed;
  /// 重力加速度（ピクセル/秒^2）
  double gravity;
  /// 体パーツ設定
  PlayerPiePartConfig body;
  /// 頭パーツ設定
  PlayerCirclePartConfig head;
  /// 手（前）パーツ設定
  PlayerCirclePartConfig handFront;
  /// 手（後）パーツ設定
  PlayerCirclePartConfig handBack;
  /// 足（左）パーツ設定
  PlayerCirclePartConfig footLeft;
  /// 足（右）パーツ設定
  PlayerCirclePartConfig footRight;

  /// @brief TOML からプレイヤー設定を生成する
  /// @param toml TOML 値
  /// @return PlayerConfig
  [[nodiscard]] static PlayerConfig FromToml(const s3d::TOMLValue& toml);
};
