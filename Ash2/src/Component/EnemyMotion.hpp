#pragma once
#include <variant>

/// @brief Enemy専用のモーション（行動状態）
namespace EnemyMotion {

/// @brief 通常状態（無反応）
struct Idle {};

/// @brief ひるみ中（縦縮み演出のみ。原寸は EnemyConfig::size から復元する）
struct Stagger {
  /// 残り時間（秒）
  double remaining = 0.0;
};

/// @brief 弾かれ中（後方へ滑るだけ）
struct Repel {
  /// 残り時間（秒）
  double remaining = 0.0;
};

/// @brief 吹っ飛び中（Velocity+Gravity による放物線は
/// MovementSystem/GravitySystem に委ねる）
struct Knockback {
  /// 残り時間（秒）
  double remaining = 0.0;
};

/// @brief 撃破後の消滅演出中（アルファフェード。満了後の破棄は EnemySystem
/// が行う）
struct Defeated {
  /// 残り時間（秒）
  double remaining = 0.0;
};

/// @brief 敵の排他的な行動状態
using Variant = std::variant<Idle, Stagger, Repel, Knockback, Defeated>;

}  // namespace EnemyMotion
