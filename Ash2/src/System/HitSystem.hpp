#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Hp.hpp"
#include "Component/Invincible.hpp"
#include "Component/WorldPos.hpp"

/// @brief 攻撃側・被弾側のエンティティの組
struct HitPair {
  entt::entity attacker;
  entt::entity target;
};

/// @brief ヒット判定システム
class HitSystem {
 public:
  /// @brief 攻撃側コライダーと被弾側コライダーの重なりを検出し、Hp
  /// にダメージを適用する
  /// @return このフレームで新たに成立したヒットの一覧
  [[nodiscard]] static Array<HitPair> Update(entt::registry& registry);

 private:
  /// @brief 線分を表す内部構造体
  struct Segment {
    Vec3 start;
    Vec3 end;
  };

  /// @brief 2線分間の最近接距離の二乗を返す
  [[nodiscard]] static double SegmentDistSq(Segment segA, Segment segB);
};

inline double HitSystem::SegmentDistSq(Segment segA, Segment segB) {
  // 線分の長さの二乗がこの値以下のとき点とみなす
  constexpr double KDegenerateThreshold = 1e-10;

  const Vec3 d1 = segA.end - segA.start;
  const Vec3 d2 = segB.end - segB.start;
  const Vec3 r = segA.start - segB.start;
  const double a = d1.dot(d1);
  const double e = d2.dot(d2);
  const double f = d2.dot(r);

  double s = 0.0;
  double t = 0.0;

  if (a <= KDegenerateThreshold && e <= KDegenerateThreshold) {
    return r.dot(r);
  }
  if (a <= KDegenerateThreshold) {
    t = Clamp(f / e, 0.0, 1.0);
  } else {
    const double c = d1.dot(r);
    if (e <= KDegenerateThreshold) {
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

inline Array<HitPair> HitSystem::Update(entt::registry& registry) {
  Array<HitPair> hits;

  auto attackers = registry.view<WorldPos, Collider, Attack>();
  auto targets =
      registry.view<WorldPos, Collider, Hp>(entt::exclude<Invincible>);

  for (auto&& [attacker, aPos, aCol, atk] : attackers.each()) {
    // root が設定されている場合はルートの Attack を参照してヒット管理する
    const auto rootEntity = (atk.root != entt::null) ? atk.root : attacker;
    auto& rootAtk = registry.get<Attack>(rootEntity);

    const Vec3 worldA{aPos.w, aPos.h, aPos.d};
    const Vec3 ap1 = worldA + aCol.segmentStart;
    const Vec3 ap2 = worldA + aCol.segmentEnd;

    for (auto&& [target, tPos, tCol, hp] : targets.each()) {
      if (attacker == target) continue;
      if (rootEntity == target) continue;
      if (rootAtk.hitTargets.contains(target)) continue;

      const Vec3 worldT{tPos.w, tPos.h, tPos.d};
      const Vec3 tp1 = worldT + tCol.segmentStart;
      const Vec3 tp2 = worldT + tCol.segmentEnd;

      const double sumR = aCol.radius + tCol.radius;
      if (SegmentDistSq(Segment{.start = ap1, .end = ap2},
                        Segment{.start = tp1, .end = tp2}) >= sumR * sumR)
        continue;

      rootAtk.hitTargets.emplace(target);
      hp.current = Max(0, hp.current - rootAtk.damage);
      hits.push_back(HitPair{.attacker = rootEntity, .target = target});
    }
  }

  return hits;
}
