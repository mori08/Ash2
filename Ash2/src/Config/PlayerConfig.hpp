#pragma once
#include <Siv3D.hpp>

/// @brief 近距離攻撃の設定値
struct MeleeConfig {
  /// 攻撃カプセルの高さ中点（WorldPos.h 方向オフセット）
  double capMidH;
  /// 攻撃リーチ（w 軸方向の距離）
  double reach;
  /// 攻撃カプセルの半径
  double radius;
  /// 与えるダメージ量
  int damage;
};

/// @brief 遠距離攻撃の設定値
struct RangedConfig {
  /// 攻撃リーチ（w 軸方向の距離）
  double reach;
  /// 攻撃カプセルの半径（弾コライダーの半径・CircleDrawable の表示半径と兼用）
  double radius;
  /// 与えるダメージ量
  int damage;
  /// 弾の移動速度（横方向、ピクセル/秒）
  double bulletSpeed;
  /// 弾の発射高さ（プレイヤーの WorldPos.h からのオフセット）
  double spawnHeight;
};

/// @brief プレイヤーの設定値
struct PlayerConfig {
  /// 横移動速度（ピクセル/秒）
  double speed;
  /// ジャンプ初速（ピクセル/秒）
  double jumpSpeed;
  /// 重力加速度（ピクセル/秒^2）
  double gravity;
  /// 近距離攻撃設定
  MeleeConfig melee;
  /// 遠距離攻撃設定
  RangedConfig ranged;

  /// @brief TOML からプレイヤー設定を生成する
  /// @param toml TOML 値
  /// @return PlayerConfig
  [[nodiscard]] static PlayerConfig FromToml(const s3d::TOMLValue& toml);
};
