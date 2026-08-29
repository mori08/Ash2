#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/WorldPos.hpp"
#include "System/AttachmentSystem.hpp"
#include "System/HierarchySystem.hpp"

// 親子関係は「親を動かしたときに子が追従するか」で検証する。
// 兄弟の並び順は実装上の都合であり、振る舞いとして保証しない。
//
// 各テストは本番（InitializeRegistry）と同じく HierarchySystem::Connect を
// 呼ぶ。破棄シグナル経由の Detach がないと、破棄済みの子がリストに残る。

namespace {

/// @brief WorldPos を持つエンティティを生成する
entt::entity MakeNode(entt::registry& registry, const WorldPos& pos = {}) {
  const auto entity = registry.create();
  registry.emplace<WorldPos>(entity, pos);
  return entity;
}

/// @brief 親の絶対座標を移動させて伝播を1回走らせる
void MoveParent(entt::registry& registry, entt::entity parent, double w) {
  registry.get<WorldPos>(parent).w = w;
  AttachmentSystem::UpdateTransform(registry);
}

}  // namespace

TEST_CASE("Hierarchy::Attach - records the given offset as LocalOffset") {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = MakeNode(registry);
  const auto child = MakeNode(registry);

  Hierarchy::Attach(
      registry, parent, child, LocalOffset{.w = 1.0, .h = 2.0, .d = 3.0}
  );

  REQUIRE(registry.all_of<LocalOffset>(child));
  const auto& offset = registry.get<const LocalOffset>(child);
  REQUIRE(offset.w == Approx(1.0));
  REQUIRE(offset.h == Approx(2.0));
  REQUIRE(offset.d == Approx(3.0));
}

TEST_CASE("Hierarchy::Attach - child follows a parent that had no Hierarchy") {
  // Hierarchy 未所持のエンティティ同士でも関係を張れる
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = MakeNode(registry);
  const auto child = MakeNode(registry);

  Hierarchy::Attach(registry, parent, child, LocalOffset{.w = 1.0});
  MoveParent(registry, parent, 100.0);

  REQUIRE(registry.get<const WorldPos>(child).w == Approx(101.0));
}

TEST_CASE("Hierarchy::Attach - every attached child follows the parent") {
  // 子が何体いても全員が追従する（並び順は問わない）
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = MakeNode(registry);
  const auto child1 = MakeNode(registry);
  const auto child2 = MakeNode(registry);
  const auto child3 = MakeNode(registry);

  Hierarchy::Attach(registry, parent, child1, LocalOffset{.w = 1.0});
  Hierarchy::Attach(registry, parent, child2, LocalOffset{.w = 2.0});
  Hierarchy::Attach(registry, parent, child3, LocalOffset{.w = 3.0});

  MoveParent(registry, parent, 100.0);

  REQUIRE(registry.get<const WorldPos>(child1).w == Approx(101.0));
  REQUIRE(registry.get<const WorldPos>(child2).w == Approx(102.0));
  REQUIRE(registry.get<const WorldPos>(child3).w == Approx(103.0));
}

TEST_CASE(
    "Hierarchy::Attach - re-attaching switches which parent the child follows"
) {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent1 = MakeNode(registry);
  const auto parent2 = MakeNode(registry);
  const auto child = MakeNode(registry);

  Hierarchy::Attach(registry, parent1, child, LocalOffset{.w = 1.0});
  Hierarchy::Attach(registry, parent2, child, LocalOffset{.w = 1.0});

  registry.get<WorldPos>(parent1).w = 100.0;
  MoveParent(registry, parent2, 500.0);

  // 旧親（100.0）ではなく新親（500.0）に追従する
  REQUIRE(registry.get<const WorldPos>(child).w == Approx(501.0));
}

TEST_CASE(
    "Hierarchy::Detach - detached child stops following and loses offset"
) {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = MakeNode(registry);
  const auto child = MakeNode(registry);

  Hierarchy::Attach(registry, parent, child, LocalOffset{.w = 1.0});
  MoveParent(registry, parent, 100.0);

  Hierarchy::Detach(registry, child);
  REQUIRE_FALSE(registry.all_of<LocalOffset>(child));

  MoveParent(registry, parent, 500.0);

  // 切り離した時点の座標に留まる
  REQUIRE(registry.get<const WorldPos>(child).w == Approx(101.0));
}

TEST_CASE("Hierarchy::Detach - the remaining children keep following") {
  // 途中の子を切り離しても、残りの子への伝播経路は壊れない
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = MakeNode(registry);
  const auto child1 = MakeNode(registry);
  const auto child2 = MakeNode(registry);
  const auto child3 = MakeNode(registry);

  Hierarchy::Attach(registry, parent, child1, LocalOffset{.w = 1.0});
  Hierarchy::Attach(registry, parent, child2, LocalOffset{.w = 2.0});
  Hierarchy::Attach(registry, parent, child3, LocalOffset{.w = 3.0});

  Hierarchy::Detach(registry, child2);
  MoveParent(registry, parent, 100.0);

  REQUIRE(registry.get<const WorldPos>(child1).w == Approx(101.0));
  REQUIRE(registry.get<const WorldPos>(child3).w == Approx(103.0));
}

TEST_CASE("Hierarchy::DestroyWithChildren - destroys the entire subtree") {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = MakeNode(registry);
  const auto child = MakeNode(registry);
  const auto grandchild = MakeNode(registry);

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
  const auto parent = MakeNode(registry);
  const auto doomed = MakeNode(registry);
  const auto survivor = MakeNode(registry);

  Hierarchy::Attach(registry, parent, doomed);
  Hierarchy::Attach(registry, parent, survivor, LocalOffset{.w = 3.0});

  Hierarchy::DestroyWithChildren(registry, doomed);

  REQUIRE_FALSE(registry.valid(doomed));
  REQUIRE(registry.valid(survivor));

  MoveParent(registry, parent, 100.0);
  REQUIRE(registry.get<const WorldPos>(survivor).w == Approx(103.0));
}

TEST_CASE(
    "Hierarchy::DestroyWithChildren - is safe on an already-destroyed entity"
) {
  entt::registry registry;
  HierarchySystem::Connect(registry);
  const auto parent = MakeNode(registry);
  const auto child = MakeNode(registry);
  Hierarchy::Attach(registry, parent, child);

  Hierarchy::DestroyWithChildren(registry, parent);
  // 破棄済みエンティティを再度渡してもクラッシュしない
  Hierarchy::DestroyWithChildren(registry, parent);
  Hierarchy::DestroyWithChildren(registry, child);

  REQUIRE_FALSE(registry.valid(parent));
  REQUIRE_FALSE(registry.valid(child));
}

#endif
