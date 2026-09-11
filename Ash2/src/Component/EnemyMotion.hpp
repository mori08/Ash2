#pragma once
#include <variant>

/// @brief Enemy専用のモーション（行動状態）
namespace EnemyMotion {

/// @brief 通常状態（索敵範囲内かつ接地中なら Chase へ移行する）
struct Idle {};

/// @brief プレイヤーへ w-d 平面を接近中
struct Chase {};

/// @brief 飛びつきの溜め
struct Windup {
  /// 残り時間（秒）
  double remaining = 0.0;
};

/// @brief 跳躍中（本体の Collider に Attack
/// が乗り体当たり判定として機能する。接地で満了）
struct Leap {};

/// @brief 着地硬直
struct Landing {
  /// 残り時間（秒）
  double remaining = 0.0;
};

/// @brief ひるみ中
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
using Variant = std::variant<
    Idle, Chase, Windup, Leap, Landing, Stagger, Repel, Knockback, Defeated>;

}  // namespace EnemyMotion
