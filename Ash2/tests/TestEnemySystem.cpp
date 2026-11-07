#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/EnemyMotion.hpp"
#include "Component/Hierarchy.hpp"
#include "System/EnemySystem.hpp"
#include "System/HierarchySystem.hpp"

namespace {

/// @brief 指定の状態の敵エンティティを生成する
entt::entity MakeEnemy(
    entt::registry& registry, const EnemyMotion::Variant& motion
) {
  const auto enemy = registry.create();
  registry.emplace<EnemyMotion::Variant>(enemy, motion);
  return enemy;
}

}  // namespace

TEST_CASE("EnemySystem - destroys entity when Defeated has expired") {
  entt::registry registry;
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Defeated{.remaining = 0.0});

  EnemySystem::Update(registry);

  REQUIRE_FALSE(registry.valid(enemy));
}

TEST_CASE(
    "EnemySystem - keeps entity while Defeated still has remaining time"
) {
  entt::registry registry;
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Defeated{.remaining = 0.5});

  EnemySystem::Update(registry);

  REQUIRE(registry.valid(enemy));
}

TEST_CASE("EnemySystem - leaves non-Defeated entities untouched") {
  entt::registry registry;
  const auto enemy = MakeEnemy(registry, EnemyMotion::Idle{});

  EnemySystem::Update(registry);

  REQUIRE(registry.valid(enemy));
}

TEST_CASE("EnemySystem - destroys the children of an expired enemy") {
  // ロックのレティクルは敵の子として付くため、敵と一緒に消える
  entt::registry registry;
  // 本番（InitializeRegistry）と同じく Hierarchy の破棄シグナルを登録する
  HierarchySystem::Connect(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Defeated{.remaining = 0.0});
  const auto child = registry.create();
  Hierarchy::Attach(registry, enemy, child);

  EnemySystem::Update(registry);

  REQUIRE_FALSE(registry.valid(enemy));
  REQUIRE_FALSE(registry.valid(child));
}

#endif
