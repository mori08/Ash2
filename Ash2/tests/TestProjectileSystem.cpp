#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Projectile.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "System/ProjectileSystem.hpp"

namespace {

/// @brief テスト用の弾エンティティを生成する
/// @param registry ECS レジストリ
/// @param pos 初期ワールド座標
/// @param vel 速度
/// @return 生成した弾エンティティ
entt::entity MakeBullet(
    entt::registry& registry, const WorldPos& pos, const Velocity& vel
) {
  const auto bullet = registry.create();
  registry.emplace<Projectile>(bullet);
  registry.emplace<WorldPos>(bullet, pos);
  registry.emplace<Velocity>(bullet, vel);
  registry.emplace<Collider>(
      bullet,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 5.0
      }
  );
  registry.emplace<Attack>(bullet, Attack{.damage = 10});
  return bullet;
}

}  // namespace

TEST_CASE(
    "ProjectileSystem - destroys bullet on impact (hitTargets non-empty)"
) {
  // Attack.hitTargets が空でなくなった（HitSystem
  // がヒットを記録した）場合は破棄される
  entt::registry registry;

  const auto bullet = MakeBullet(
      registry, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0},
      Velocity{.w = 100.0, .h = 0.0, .d = 0.0}
  );

  // HitSystem が記録したことを模してダミーのターゲットを hitTargets に追加
  const auto dummyTarget = registry.create();
  registry.get<Attack>(bullet).hitTargets.emplace(dummyTarget);

  ProjectileSystem::Update(registry);

  REQUIRE_FALSE(registry.valid(bullet));
}

#endif
