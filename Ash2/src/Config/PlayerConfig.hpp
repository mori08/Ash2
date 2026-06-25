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
  /// 構え時間（秒）
  double windupSec;
  /// 攻撃判定が有効な時間（秒）
  double activeSec;
  /// 後隙時間（秒）
  double recoverySec;
  /// 2段目の攻撃判定が有効な時間（秒）
  double active2Sec;
  /// 2段目の斬り上げの振り幅（capMidH を中心とした上下の幅）
  double slashRiseHeight;
  /// 3段目の構え時間（秒）
  double windup3Sec;
  /// 3段目の攻撃判定が有効な時間（秒）
  double active3Sec;
  /// 3段目の後隙時間（秒）
  double recovery3Sec;
  /// 3段目の攻撃カプセルの半径
  double radius3;
};

/// @brief ダッシュの設定値
struct DashConfig {
  /// ダッシュ移動速度（ピクセル/秒）
  double speed;
  /// 構え時間（秒）
  double windupSec;
  /// ダッシュ時間（秒）
  double dashSec;
  /// 後隙A（キャンセル不可）の時間（秒）
  double recoveryASec;
  /// 後隙B（キャンセル可）の時間（秒）
  double recoveryBSec;
  /// 1回の発生に必要なスタミナ消費量
  int staminaCost;
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
  MeleeConfig melee;
  RangedConfig ranged;
  DashConfig dash;

  /// @brief TOML からプレイヤー設定を生成する
  [[nodiscard]] static PlayerConfig FromToml(const s3d::TOMLValue& toml);
};
