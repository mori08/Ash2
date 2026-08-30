#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/ReactionLevel.hpp"

/// @brief 成立したヒット1件の情報
///
/// 攻撃側のエンティティ自体は保持しない。プレイヤーの近接ヒットボックスは
/// 被弾処理の中で `Attack` を外されうるため、後で引き直さずに済むよう
/// ヒット成立時点で必要な値を確定させて写す。
struct HitEvent {
  /// 被弾側
  entt::entity target;
  /// 攻撃側の本体（ヒットボックスの Hierarchy 親。親を持たなければ攻撃側自身）
  entt::entity attackerOwner;
  /// ヒット成立時点の Attack::hitstopSec の写し
  double hitstopSec;
  /// ヒット成立時点の Attack::reaction の写し
  ReactionLevel reaction;
};

/// @brief ヒット判定システム
class HitSystem {
 public:
  /// @brief 攻撃側コライダーと被弾側コライダーの重なりを検出し、Hp
  /// にダメージを適用する
  /// @return このフレームで新たに成立したヒットの一覧
  [[nodiscard]] static Array<HitEvent> Update(entt::registry& registry);
};
