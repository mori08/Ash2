#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

struct FrameData;

/// @brief プレイヤーの移動・ジャンプ・重力・アニメーションクリップ選択システム
class PlayerMovementSystem {
 public:
  /// @brief Player コンポーネントを持つエンティティの移動処理を更新する
  ///
  /// 水平速度の決定・位置への速度適用・ジャンプ・重力・地面クランプ・
  /// アニメーションクリップ選択・向き更新を行う。`NeutralState`
  /// を持つエンティティのみ移動・ジャンプを許可し、持たない場合は
  /// `AttackState.clip` をアニメーションクリップとして使用する。
  static void Update(entt::registry& registry, const FrameData& frameData);
};
