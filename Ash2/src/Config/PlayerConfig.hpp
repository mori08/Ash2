#pragma once
#include <Siv3D.hpp>

/// @brief プレイヤーテクスチャのフレーム設定
struct PlayerTextureConfig {
  /// スプライトシート上のフレーム幅（ピクセル）
  int frameWidth;
  /// スプライトシート上のフレーム高さ（ピクセル）
  int frameHeight;
  /// 使用するフレームインデックス（0 始まり、左上から横方向）
  int frameIndex;
  /// 描画オフセット（中心座標からのずれ、アンカー調整用）
  s3d::Vec2 drawOffset;
};

/// @brief プレイヤーの設定値
struct PlayerConfig {
  /// 横移動速度（ピクセル/秒）
  double speed;
  /// ジャンプ初速（ピクセル/秒）
  double jumpSpeed;
  /// 重力加速度（ピクセル/秒^2）
  double gravity;
  /// テクスチャフレーム設定
  PlayerTextureConfig texture;

  /// @brief TOML からプレイヤー設定を生成する
  /// @param toml TOML 値
  /// @return PlayerConfig
  [[nodiscard]] static PlayerConfig FromToml(const s3d::TOMLValue& toml);
};
