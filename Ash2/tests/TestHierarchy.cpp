#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "System/HierarchySystem.hpp"

// 親子関係は Hierarchy::Children で直接観測する。
// 兄弟の並び順は実装上の都合であり、振る舞いとして保証しない。
//
// 各テストは本番（InitializeRegistry）と同じく HierarchySystem::Connect を
// 呼ぶ。破棄シグナル経由の Detach がないと、破棄済みの子がリストに残る。

namespace {

/// @brief 親の子を収集する（順序は保証しない）
Array<entt::entity> ChildrenOf(
    const entt::registry& registry, entt::entity parent
) {
  return Hierarchy::Children(registry, parent) |
         std::ranges::to<Array<entt::entity>>();
}

}  // namespace

TEST_CASE("Hierarchy::Attach - records the given offset as LocalOffset") {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto child = registry.create();

  Hierarchy::Attach(
      registry, parent, child, LocalOffset{.w = 1.0, .h = 2.0, .d = 3.0}
  );

  REQUIRE(registry.all_of<LocalOffset>(child));
  const auto& offset = registry.get<const LocalOffset>(child);
  REQUIRE(offset.w == Approx(1.0));
  REQUIRE(offset.h == Approx(2.0));
  REQUIRE(offset.d == Approx(3.0));
}

TEST_CASE("Hierarchy::Attach - parent without Hierarchy gets the child") {
  // Hierarchy 未所持のエンティティ同士でも関係を張れる
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto child = registry.create();

  Hierarchy::Attach(registry, parent, child);

  const auto children = ChildrenOf(registry, parent);
  REQUIRE(children.size() == 1);
  REQUIRE(children.contains(child));
}

TEST_CASE("Hierarchy::Attach - every attached child appears exactly once") {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto child1 = registry.create();
  const auto child2 = registry.create();
  const auto child3 = registry.create();

  Hierarchy::Attach(registry, parent, child1);
  Hierarchy::Attach(registry, parent, child2);
  Hierarchy::Attach(registry, parent, child3);

  const auto children = ChildrenOf(registry, parent);
  REQUIRE(children.size() == 3);
  REQUIRE(children.contains(child1));
  REQUIRE(children.contains(child2));
  REQUIRE(children.contains(child3));
}

TEST_CASE(
    "Hierarchy::Attach - re-attaching moves the child to the new parent"
) {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent1 = registry.create();
  const auto parent2 = registry.create();
  const auto child = registry.create();

  Hierarchy::Attach(registry, parent1, child);
  Hierarchy::Attach(registry, parent2, child);

  REQUIRE(ChildrenOf(registry, parent1).empty());
  const auto children = ChildrenOf(registry, parent2);
  REQUIRE(children.size() == 1);
  REQUIRE(children.contains(child));
}

TEST_CASE("Hierarchy::Detach - detached child is removed and loses offset") {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto child = registry.create();

  Hierarchy::Attach(registry, parent, child, LocalOffset{.w = 1.0});
  Hierarchy::Detach(registry, child);

  REQUIRE_FALSE(ChildrenOf(registry, parent).contains(child));
  REQUIRE_FALSE(registry.all_of<LocalOffset>(child));
}

TEST_CASE("Hierarchy::Detach - the remaining children stay") {
  // 途中の子を切り離しても、残りの子への連結は壊れない
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto child1 = registry.create();
  const auto child2 = registry.create();
  const auto child3 = registry.create();

  Hierarchy::Attach(registry, parent, child1);
  Hierarchy::Attach(registry, parent, child2);
  Hierarchy::Attach(registry, parent, child3);

  Hierarchy::Detach(registry, child2);

  const auto children = ChildrenOf(registry, parent);
  REQUIRE(children.size() == 2);
  REQUIRE(children.contains(child1));
  REQUIRE(children.contains(child3));
}

TEST_CASE("Hierarchy::DestroyWithChildren - destroys the entire subtree") {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto child = registry.create();
  const auto grandchild = registry.create();

  Hierarchy::Attach(registry, parent, child);
  Hierarchy::Attach(registry, child, grandchild);

  Hierarchy::DestroyWithChildren(registry, parent);

  REQUIRE_FALSE(registry.valid(parent));
  REQUIRE_FALSE(registry.valid(child));
  REQUIRE_FALSE(registry.valid(grandchild));
}

TEST_CASE("Hierarchy::DestroyWithChildren - leaves siblings of the subtree") {
  // 部分木だけを破棄し、親に残る他の子は生き続ける
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto doomed = registry.create();
  const auto survivor = registry.create();

  Hierarchy::Attach(registry, parent, doomed);
  Hierarchy::Attach(registry, parent, survivor);

  Hierarchy::DestroyWithChildren(registry, doomed);

  REQUIRE_FALSE(registry.valid(doomed));
  REQUIRE(registry.valid(survivor));
  const auto children = ChildrenOf(registry, parent);
  REQUIRE(children.size() == 1);
  REQUIRE(children.contains(survivor));
}

TEST_CASE("Hierarchy::Children - is empty when the parent has no Hierarchy") {
  entt::registry registry;
  const auto parent = registry.create();

  REQUIRE(Hierarchy::Children(registry, parent).empty());
}

TEST_CASE("Hierarchy::Children - is empty after every child is detached") {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto child1 = registry.create();
  const auto child2 = registry.create();

  Hierarchy::Attach(registry, parent, child1);
  Hierarchy::Attach(registry, parent, child2);
  Hierarchy::Detach(registry, child1);
  Hierarchy::Detach(registry, child2);

  REQUIRE(Hierarchy::Children(registry, parent).empty());
}

TEST_CASE("Hierarchy::Children - throws when the current child is detached") {
  // 走査中に今の子を Detach すると、黙って途中で終わらず throw する
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = registry.create();
  const auto child1 = registry.create();
  const auto child2 = registry.create();

  Hierarchy::Attach(registry, parent, child1);
  Hierarchy::Attach(registry, parent, child2);

  const auto detachAndAdvance = [&] {
    auto it = Hierarchy::Children(registry, parent).begin();
    Hierarchy::Detach(registry, *it);
    ++it;
  };
  REQUIRE_THROWS_AS(detachAndAdvance(), Error);
}

#endif
