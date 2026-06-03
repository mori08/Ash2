#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Hp.hpp"
#include "Component/WorldPos.hpp"
#include "System/HitSystem.hpp"

TEST_CASE("HitSystem - no repeated damage from multi-frame attack") {
  // 複数フレーム持続する攻撃で2回目以降ダメージが入らない
  entt::registry registry;

  // 攻撃エンティティ（WorldPos 原点、半径10の球コライダー）
  auto attacker = registry.create();
  registry.emplace<WorldPos>(attacker, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(attacker, Collider{.segmentStart = {0.0, 0.0, 0.0},
                                                .segmentEnd = {0.0, 0.0, 0.0},
                                                .radius = 10.0});
  registry.emplace<Attack>(attacker, Attack{.damage = 5});

  // 被弾エンティティ（隣接、重なる位置）
  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(target, Collider{.segmentStart = {0.0, 0.0, 0.0},
                                              .segmentEnd = {0.0, 0.0, 0.0},
                                              .radius = 10.0});
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});

  // 1フレーム目：ヒットしてダメージが入る
  HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 95);

  // 2フレーム目：同じ攻撃が持続しているが hitTargets
  // に登録済みなのでダメージなし
  HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 95);

  // 3フレーム目：同上
  HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 95);
}

TEST_CASE("HitSystem - multi-collider attack hits target only once") {
  // 複数コライダー構成でターゲットへのダメージが1回だけ
  entt::registry registry;

  // ルートエンティティ（Attack を持つ、Collider は持たない想定だが root
  // 参照用に Attack を保持）
  auto root = registry.create();
  registry.emplace<Attack>(root, Attack{.damage = 10});

  // 子コライダー1（root を参照）
  auto child1 = registry.create();
  registry.emplace<WorldPos>(child1, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(child1, Collider{.segmentStart = {0.0, 0.0, 0.0},
                                              .segmentEnd = {0.0, 0.0, 0.0},
                                              .radius = 10.0});
  registry.emplace<Attack>(child1, Attack{.damage = 10, .root = root});

  // 子コライダー2（同じ root を参照）
  auto child2 = registry.create();
  registry.emplace<WorldPos>(child2, WorldPos{.w = 3.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(child2, Collider{.segmentStart = {0.0, 0.0, 0.0},
                                              .segmentEnd = {0.0, 0.0, 0.0},
                                              .radius = 10.0});
  registry.emplace<Attack>(child2, Attack{.damage = 10, .root = root});

  // 被弾エンティティ（両コライダーと重なる位置）
  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(target, Collider{.segmentStart = {0.0, 0.0, 0.0},
                                              .segmentEnd = {0.0, 0.0, 0.0},
                                              .radius = 10.0});
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});

  // 1回の Update で child1 と child2 が両方ヒットしても、ダメージは1回分のみ
  HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 90);
}

#endif
