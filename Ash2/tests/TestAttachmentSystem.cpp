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
      registry, parent, child, WorldPos{.w = 1.0, .h = 0.5, .d = 2.0}
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
  Hierarchy::Attach(registry, parent, child, WorldPos{.w = 1.0});

  auto grandchild = registry.create();
  registry.emplace<WorldPos>(grandchild);
  Hierarchy::Attach(registry, child, grandchild, WorldPos{.w = 1.0});

  AttachmentSystem::UpdateTransform(registry);

  REQUIRE(registry.get<const WorldPos>(child).w == 11.0);
  REQUIRE(registry.get<const WorldPos>(grandchild).w == 12.0);
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
