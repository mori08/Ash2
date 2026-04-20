#if USE_TEST
#include <ThirdParty/Catch2/catch.hpp>
#include <ThirdParty/entt/entt.hpp>

#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/WorldPos.hpp"
#include "System/AttachmentSystem.hpp"

TEST_CASE("AttachmentSystem - child follows parent with offset") {
  entt::registry registry;

  auto parent = registry.create();
  registry.emplace<WorldPos>(parent, WorldPos{.w = 10.0, .h = 2.0, .d = 5.0});

  auto child = registry.create();
  registry.emplace<WorldPos>(child);
  Hierarchy::Attach(registry, parent, child,
                    WorldPos{.w = 1.0, .h = 0.5, .d = 2.0});

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

TEST_CASE("AttachmentSystem - DestroyWithChildren destroys entire subtree") {
  entt::registry registry;

  auto parent = registry.create();
  registry.emplace<WorldPos>(parent);

  auto child = registry.create();
  registry.emplace<WorldPos>(child);
  Hierarchy::Attach(registry, parent, child);

  auto grandchild = registry.create();
  registry.emplace<WorldPos>(grandchild);
  Hierarchy::Attach(registry, child, grandchild);

  Hierarchy::DestroyWithChildren(registry, parent);

  REQUIRE_FALSE(registry.valid(parent));
  REQUIRE_FALSE(registry.valid(child));
  REQUIRE_FALSE(registry.valid(grandchild));
}

TEST_CASE("AttachmentSystem - Attach sets up linked list correctly") {
  entt::registry registry;

  auto parent = registry.create();
  registry.emplace<WorldPos>(parent);

  auto child1 = registry.create();
  registry.emplace<WorldPos>(child1);
  Hierarchy::Attach(registry, parent, child1);

  auto child2 = registry.create();
  registry.emplace<WorldPos>(child2);
  Hierarchy::Attach(registry, parent, child2);

  // child2 は先頭挿入なので firstChild == child2
  const auto& parentNode = registry.get<const Hierarchy>(parent);
  REQUIRE(parentNode.firstChild() == child2);
  REQUIRE(registry.get<const Hierarchy>(child2).nextSibling() == child1);
  REQUIRE(registry.get<const Hierarchy>(child1).prevSibling() == child2);
}

TEST_CASE("AttachmentSystem - Detach removes child from linked list") {
  entt::registry registry;

  auto parent = registry.create();
  registry.emplace<WorldPos>(parent);

  auto child = registry.create();
  registry.emplace<WorldPos>(child);
  Hierarchy::Attach(registry, parent, child);

  Hierarchy::Detach(registry, child);

  REQUIRE(registry.get<const Hierarchy>(parent).firstChild() ==
          entt::entity{entt::null});
  REQUIRE(registry.get<const Hierarchy>(child).parent() ==
          entt::entity{entt::null});
  REQUIRE_FALSE(registry.all_of<LocalOffset>(child));
}

#endif
