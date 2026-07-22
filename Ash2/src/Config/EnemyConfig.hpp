#pragma once
#include <Siv3D.hpp>

/// @brief 敵の設定値
struct EnemyConfig {
  /// 最大HP
  int32 maxHp;
  /// 描画サイズ（幅・高さ）
  SizeF size;
  /// 当たり判定カプセルの半径
  double capsuleRadius;
  /// 当たり判定カプセルの高さ（足元からの縦カプセル）
  double capsuleHeight;
  /// 生成時の初期横位置（WorldPos.w）
  double spawnW;

  /// ひるみ（Stagger）の演出時間（秒）
  double staggerSec;

  /// 弾き（Repel）の後方移動速度（ピクセル/秒）
  double repelSpeed;
  /// 弾き（Repel）の持続時間（秒）
  double repelSec;

  /// 吹っ飛び（Knockback）の横方向初速（ピクセル/秒）
  double blowSpeedW;
  /// 吹っ飛び（Knockback）の垂直方向初速（ピクセル/秒）
  double blowSpeedH;
  /// 吹っ飛び（Knockback）の持続時間（秒）
  double knockbackSec;

  /// 撃破後の消滅演出時間（秒）
  double defeatedSec;
  /// 撃破後、再出現までの待機時間（秒）
  double respawnSec;

  /// @brief TOML から敵設定を生成する
  [[nodiscard]] static EnemyConfig FromToml(const TOMLValue& toml);
};
