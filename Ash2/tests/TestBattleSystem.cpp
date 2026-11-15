#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/FadeOut.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Projectile.hpp"
#include "System/BattleSystem.hpp"
#include "System/HierarchySystem.hpp"

TEST_CASE("BattleSystem::Cleanup - destroys entities with Projectile") {
  entt::registry registry;
  // 本番（InitializeRegistry）と同じく Hierarchy の破棄シグナルを登録する
  HierarchySystem::Connect(registry);
  const auto projectile = registry.create();
  registry.emplace<Projectile>(projectile);

  BattleSystem::Cleanup(registry);

  REQUIRE_FALSE(registry.valid(projectile));
}

TEST_CASE("BattleSystem::Cleanup - destroys entities with FadeOut") {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto fading = registry.create();
  registry.emplace<FadeOut>(fading, FadeOut{.duration = 0.5, .remaining = 0.5});
  // 本番では Detach 後も Hierarchy が残るため、同じ状態にしておく
  registry.emplace<Hierarchy>(fading);

  BattleSystem::Cleanup(registry);

  REQUIRE_FALSE(registry.valid(fading));
}

TEST_CASE(
    "BattleSystem::Cleanup - keeps entities without Projectile or FadeOut"
) {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto other = registry.create();

  BattleSystem::Cleanup(registry);

  REQUIRE(registry.valid(other));
}

#endif
