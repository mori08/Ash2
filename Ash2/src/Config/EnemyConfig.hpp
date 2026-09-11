#pragma once
#include <Siv3D.hpp>

#include <expected>

#include "Config/ReactionLevel.hpp"

/// @brief 敵の設定値
struct EnemyConfig {
  /// 最大HP
  int32 maxHp;
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

  /// Chase の接近速度（ピクセル/秒）
  double moveSpeed;
  /// Idle から Chase へ移行する索敵距離（w-d 平面）
  double aggroRange;
  /// Chase から Windup へ移行する飛びつき開始距離（w-d 平面）
  double leapRange;

  /// Windup の溜め時間（秒）
  double windupSec;
  /// Leap の初速（横方向、w-d 平面のプレイヤー方向）
  double leapSpeedW;
  /// Leap の初速（垂直方向）
  double leapSpeedH;
  /// Landing の着地硬直時間（秒）
  double landingSec;

  /// Leap の体当たりが与えるダメージ量
  int32 attackDamage;
  /// Leap の体当たりのヒット成立時に付与するヒットストップ時間（秒）
  double attackHitstopSec;
  /// Leap の体当たりが被弾側に生じさせるリアクションの強さ
  ReactionLevel attackReaction;

  /// @brief TOML から敵設定を生成する
  [[nodiscard]] static std::expected<EnemyConfig, String> FromToml(
      const TOMLValue& toml
  );
};
