#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Hierarchy.hpp"
#include "Component/WorldPos.hpp"
#include "System/AttachmentSystem.hpp"

TEST_CASE("AttachmentSystem - child follows parent with offset") {
  entt::registry registry;

  auto parent = registry.create();
  registry.emplace<WorldPos>(parent, WorldPos{.w = 10.0, .h = 2.0, .d = 5.0});

  auto child = registry.create();
  registry.emplace<WorldPos>(child);
  Hierarchy::Attach(
      registry, parent, child, LocalOffset{.w = 1.0, .h = 0.5, .d = 2.0}
  );

  AttachmentSystem::UpdateTransform(registry);

  const auto& pos = registry.get<const WorldPos>(child);
  REQUIRE(pos.w == 11.0);
  REQUIRE(pos.h == 2.5);
  REQUIRE(pos.d == 7.0);
}

TEST_CASE("AttachmentSystem - grandchild is updated correctly") {
  entt::registry registry;

  auto parent = registry.create();
  registry.emplace<WorldPos>(parent, WorldPos{.w = 10.0});

  auto child = registry.create();
  registry.emplace<WorldPos>(child);
  Hierarchy::Attach(registry, parent, child, LocalOffset{.w = 1.0});

  auto grandchild = registry.create();
  registry.emplace<WorldPos>(grandchild);
  Hierarchy::Attach(registry, child, grandchild, LocalOffset{.w = 1.0});

  AttachmentSystem::UpdateTransform(registry);

  REQUIRE(registry.get<const WorldPos>(child).w == 11.0);
  REQUIRE(registry.get<const WorldPos>(grandchild).w == 12.0);
}

TEST_CASE("AttachmentSystem - every sibling follows the parent") {
  entt::registry registry;

  auto parent = registry.create();
  registry.emplace<WorldPos>(parent, WorldPos{.w = 100.0});

  auto child1 = registry.create();
  auto child2 = registry.create();
  auto child3 = registry.create();
  registry.emplace<WorldPos>(child1);
  registry.emplace<WorldPos>(child2);
  registry.emplace<WorldPos>(child3);
  Hierarchy::Attach(registry, parent, child1, LocalOffset{.w = 1.0});
  Hierarchy::Attach(registry, parent, child2, LocalOffset{.w = 2.0});
  Hierarchy::Attach(registry, parent, child3, LocalOffset{.w = 3.0});

  AttachmentSystem::UpdateTransform(registry);

  REQUIRE(registry.get<const WorldPos>(child1).w == 101.0);
  REQUIRE(registry.get<const WorldPos>(child2).w == 102.0);
  REQUIRE(registry.get<const WorldPos>(child3).w == 103.0);
}

TEST_CASE("AttachmentSystem - zero offset child is at parent position") {
  entt::registry registry;

  auto parent = registry.create();
  registry.emplace<WorldPos>(parent, WorldPos{.w = 5.0, .h = 3.0, .d = 2.0});

  auto child = registry.create();
  registry.emplace<WorldPos>(child);
  Hierarchy::Attach(registry, parent, child);

  AttachmentSystem::UpdateTransform(registry);

  const auto& pos = registry.get<const WorldPos>(child);
  REQUIRE(pos.w == 5.0);
  REQUIRE(pos.h == 3.0);
  REQUIRE(pos.d == 2.0);
}

#endif
