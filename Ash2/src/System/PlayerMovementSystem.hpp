#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

struct FrameData;

/// @brief プレイヤーの水平移動・ジャンプ・アニメーションクリップ選択システム
class PlayerMovementSystem {
 public:
  /// @brief Player コンポーネントを持つエンティティの移動処理を更新する
  ///
  /// 水平速度の決定・ジャンプ判定（`vel.h` への代入）・
  /// アニメーションクリップ選択・向き更新を行う。`NeutralState`
  /// を持つエンティティのみ移動・ジャンプを許可し、持たない場合は
  /// `AttackState.clip` をアニメーションクリップとして使用する。
  /// 位置更新・重力・地面クランプは `MovementSystem`/`GravitySystem`
  /// が行うため、このシステムより後に呼び出すこと。
  static void Update(entt::registry& registry, const FrameData& frameData);
};
