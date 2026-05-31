#pragma once
#include <Siv3D.hpp>

#include <algorithm>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Hp.hpp"
#include "Component/WorldPos.hpp"
#include "Debug.hpp"

/// @brief ヒット判定システム
class HitSystem {
 public:
  /// @brief 攻撃側コライダーと被弾側コライダーの重なりを検出し、Hp
  /// にダメージを適用する
  /// @param registry ECS レジストリ
  static void Update(entt::registry& registry);

 private:
  /// @brief 線分を表す内部構造体
  struct Segment {
    /// 始点
    s3d::Vec3 start;
    /// 終点
    s3d::Vec3 end;
  };

  /// @brief エンティティID のシフト量（上位32bit に attacker を格納するため）
  static constexpr int KEntityIdShift = 32;

  /// @brief 2線分間の距離の二乗を返す
  /// @param segA 線分1
  /// @param segB 線分2
  /// @return 最近接距離の二乗
  [[nodiscard]] static double SegmentDistSq(Segment segA, Segment segB);

  /// @brief attacker と target のペアを uint64_t キーに変換する
  /// @param attacker 攻撃側エンティティ
  /// @param target 被弾側エンティティ
  /// @return 上位32bit=attacker、下位32bit=target の uint64_t
  [[nodiscard]] static uint64_t PairKey(entt::entity attacker,
                                        entt::entity target);
};

inline uint64_t HitSystem::PairKey(entt::entity attacker, entt::entity target) {
  return (static_cast<uint64_t>(entt::to_integral(attacker))
          << KEntityIdShift) |
         static_cast<uint64_t>(entt::to_integral(target));
}

inline double HitSystem::SegmentDistSq(Segment segA, Segment segB) {
  // 線分の長さの二乗がこの値以下のとき点とみなす
  constexpr double KDegenerateThreshold = 1e-10;

  const s3d::Vec3 d1 = segA.end - segA.start;
  const s3d::Vec3 d2 = segB.end - segB.start;
  const s3d::Vec3 r = segA.start - segB.start;
  const double a = d1.dot(d1);
  const double e = d2.dot(d2);
  const double f = d2.dot(r);

  double s = 0.0;
  double t = 0.0;

  if (a <= KDegenerateThreshold && e <= KDegenerateThreshold) {
    return r.dot(r);
  }
  if (a <= KDegenerateThreshold) {
    t = std::clamp(f / e, 0.0, 1.0);
  } else {
    const double c = d1.dot(r);
    if (e <= KDegenerateThreshold) {
      s = std::clamp(-c / a, 0.0, 1.0);
    } else {
      const double b = d1.dot(d2);
      const double denom = a * e - b * b;
      s = (denom != 0.0) ? std::clamp((b * f - c * e) / denom, 0.0, 1.0) : 0.0;
      t = (b * s + f) / e;
      if (t < 0.0) {
        t = 0.0;
        s = std::clamp(-c / a, 0.0, 1.0);
      } else if (t > 1.0) {
        t = 1.0;
        s = std::clamp((b - c) / a, 0.0, 1.0);
      }
    }
  }

  const s3d::Vec3 closest1 = segA.start + d1 * s;
  const s3d::Vec3 closest2 = segB.start + d2 * t;
  return (closest1 - closest2).dot(closest1 - closest2);
}

inline void HitSystem::Update(entt::registry& registry) {
  // attacker と target のペアを uint64_t キーで管理（重複ヒット防止）
  s3d::HashSet<uint64_t> processed;

  auto attackers = registry.view<WorldPos, Collider, Attack>();
  auto targets = registry.view<WorldPos, Collider, Hp>();

  for (auto&& [attacker, aPos, aCol, atk] : attackers.each()) {
    const s3d::Vec3 worldA{aPos.w, aPos.h, aPos.d};
    const s3d::Vec3 ap1 = worldA + aCol.segmentStart;
    const s3d::Vec3 ap2 = worldA + aCol.segmentEnd;

    for (auto&& [target, tPos, tCol, hp] : targets.each()) {
      if (attacker == target) continue;
      if (processed.contains(PairKey(attacker, target))) continue;

      const s3d::Vec3 worldT{tPos.w, tPos.h, tPos.d};
      const s3d::Vec3 tp1 = worldT + tCol.segmentStart;
      const s3d::Vec3 tp2 = worldT + tCol.segmentEnd;

      const double sumR = aCol.radius + tCol.radius;
      if (SegmentDistSq(Segment{.start = ap1, .end = ap2},
                        Segment{.start = tp1, .end = tp2}) >= sumR * sumR)
        continue;

      hp.current = std::max(0, hp.current - atk.damage);
      processed.emplace(PairKey(attacker, target));
      APP_LOG(U"Hit! HP: " + Format(hp.current) + U"/" + Format(hp.max));
    }
  }
}
