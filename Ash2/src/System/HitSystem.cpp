#include "System/HitSystem.hpp"

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hp.hpp"
#include "Component/Invincible.hpp"
#include "Component/Team.hpp"
#include "Component/WorldPos.hpp"

namespace {
/// @brief 線分を表す内部構造体
struct Segment {
  Vec3 start;
  Vec3 end;
};

/// @brief 双方が Team を持ち、値が等しいか
///
/// 片方でも Team を持たない場合は false（陣営の判定に参加しない）
[[nodiscard]] bool IsSameTeam(const Team* a, const Team* b) {
  return a != nullptr && b != nullptr && *a == *b;
}

/// @brief 2線分間の最近接距離の二乗を返す
[[nodiscard]] double SegmentDistSq(Segment segA, Segment segB) {
  // 線分の長さの二乗がこの値以下のとき点とみなす
  constexpr double kDegenerateThreshold = 1e-10;

  const Vec3 d1 = segA.end - segA.start;
  const Vec3 d2 = segB.end - segB.start;
  const Vec3 r = segA.start - segB.start;
  const double a = d1.dot(d1);
  const double e = d2.dot(d2);
  const double f = d2.dot(r);

  double s = 0.0;
  double t = 0.0;

  if (a <= kDegenerateThreshold && e <= kDegenerateThreshold) {
    return r.dot(r);
  }
  if (a <= kDegenerateThreshold) {
    t = Clamp(f / e, 0.0, 1.0);
  } else {
    const double c = d1.dot(r);
    if (e <= kDegenerateThreshold) {
      s = Clamp(-c / a, 0.0, 1.0);
    } else {
      const double b = d1.dot(d2);
      const double denom = a * e - b * b;
      s = (denom != 0.0) ? Clamp((b * f - c * e) / denom, 0.0, 1.0) : 0.0;
      t = (b * s + f) / e;
      if (t < 0.0) {
        t = 0.0;
        s = Clamp(-c / a, 0.0, 1.0);
      } else if (t > 1.0) {
        t = 1.0;
        s = Clamp((b - c) / a, 0.0, 1.0);
      }
    }
  }

  const Vec3 closest1 = segA.start + d1 * s;
  const Vec3 closest2 = segB.start + d2 * t;
  return (closest1 - closest2).dot(closest1 - closest2);
}
}  // namespace

Array<HitEvent> HitSystem::Update(entt::registry& registry) {
  Array<HitEvent> hits;

  auto attackers = registry.view<WorldPos, Collider, Attack>();
  auto targets =
      registry.view<WorldPos, Collider, Hp>(entt::exclude<Invincible>);

  for (auto&& [attacker, aPos, aCol, atk] : attackers.each()) {
    // root が設定されている場合はルートの Attack を参照してヒット管理する
    const auto rootEntity = (atk.root != entt::null) ? atk.root : attacker;
    // root は破棄済み・Attack 非保持の可能性があるため参照前に検証する
    // （attacker 自身が root の場合は attackers
    // ビューの走査対象なので必ず有効） valid
    // を先に評価する短絡順序を維持し、破棄済みエンティティへの all_of/get
    // 呼び出し（未定義動作）を避ける
    if (!registry.valid(rootEntity) || !registry.all_of<Attack>(rootEntity))
      continue;
    auto& rootAtk = registry.get<Attack>(rootEntity);
    const auto* attackerTeam = registry.try_get<Team>(attacker);

    // 攻撃側本体（ヒットボックスの Hierarchy 親）を解決する。ヒットストップ
    // 付与と被弾側リアクションの位置判定の両方で使うため、HitEvent
    // に写しを持たせてヒット成立時点で確定させる（被弾処理の途中で攻撃側の
    // Attack が外れても、後続の反復で引き直さずに済む）
    entt::entity attackerOwner = rootEntity;
    if (const auto* hierarchy = registry.try_get<Hierarchy>(rootEntity);
        hierarchy != nullptr && hierarchy->parent() != entt::null) {
      attackerOwner = hierarchy->parent();
    }

    const Vec3 worldA{aPos.w, aPos.h, aPos.d};
    const Vec3 ap1 = worldA + aCol.segmentStart;
    const Vec3 ap2 = worldA + aCol.segmentEnd;

    for (auto&& [target, tPos, tCol, hp] : targets.each()) {
      if (attacker == target) continue;
      if (rootEntity == target) continue;
      if (IsSameTeam(attackerTeam, registry.try_get<Team>(target))) continue;
      if (rootAtk.hitTargets.contains(target)) continue;

      const Vec3 worldT{tPos.w, tPos.h, tPos.d};
      const Vec3 tp1 = worldT + tCol.segmentStart;
      const Vec3 tp2 = worldT + tCol.segmentEnd;

      const double sumR = aCol.radius + tCol.radius;
      if (SegmentDistSq(
              Segment{.start = ap1, .end = ap2},
              Segment{.start = tp1, .end = tp2}
          ) >= sumR * sumR)
        continue;

      rootAtk.hitTargets.emplace(target);
      hp.current = Max(0, hp.current - rootAtk.damage);
      hits.push_back(
          HitEvent{
              .target = target,
              .attackerOwner = attackerOwner,
              .hitstopSec = rootAtk.hitstopSec,
              .reaction = rootAtk.reaction,
          }
      );
    }
  }

  return hits;
}
