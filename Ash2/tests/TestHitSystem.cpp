#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hp.hpp"
#include "Component/Team.hpp"
#include "Component/WorldPos.hpp"
#include "Config/ReactionLevel.hpp"
#include "System/HitSystem.hpp"

TEST_CASE("HitSystem - no repeated damage from multi-frame attack") {
  // 複数フレーム持続する攻撃で2回目以降ダメージが入らない
  entt::registry registry;

  // 攻撃エンティティ（WorldPos 原点、半径10の球コライダー）
  auto attacker = registry.create();
  registry.emplace<WorldPos>(attacker, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      attacker,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(attacker, Attack{.damage = 5});

  // 被弾エンティティ（隣接、重なる位置）
  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      target,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});

  // 1フレーム目：ヒットしてダメージが入り、HitEvent が1件返る
  // （attacker は Hierarchy を持たないため attackerOwner は自分自身）
  auto hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 95);
  REQUIRE(hits.size() == 1);
  REQUIRE(hits[0].attackerOwner == attacker);
  REQUIRE(hits[0].target == target);

  // 2フレーム目：同じ攻撃が持続しているが hitTargets
  // に登録済みなのでダメージなし、HitEvent も返らない
  hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 95);
  REQUIRE(hits.isEmpty());

  // 3フレーム目：同上
  hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 95);
  REQUIRE(hits.isEmpty());
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
  registry.emplace<Collider>(
      child1,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(child1, Attack{.damage = 10, .root = root});

  // 子コライダー2（同じ root を参照）
  auto child2 = registry.create();
  registry.emplace<WorldPos>(child2, WorldPos{.w = 3.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      child2,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(child2, Attack{.damage = 10, .root = root});

  // 被弾エンティティ（両コライダーと重なる位置）
  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      target,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});

  // 1回の Update で child1 と child2 が両方ヒットしても、ダメージは1回分のみ
  const auto hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 90);
  REQUIRE(hits.size() == 1);
}

TEST_CASE("HitSystem - collider with destroyed root is skipped") {
  // 破棄済みの root
  // を参照するコライダーはスキップされ、クラッシュもダメージもない
  entt::registry registry;

  // 破棄済みの root エンティティ（生成直後に破棄し、無効な entity にする）
  auto root = registry.create();
  registry.destroy(root);

  // 攻撃コライダー（無効な root を参照）
  auto attacker = registry.create();
  registry.emplace<WorldPos>(attacker, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      attacker,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(attacker, Attack{.damage = 5, .root = root});

  // 被弾エンティティ（重なる位置）
  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      target,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});

  const auto hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 100);
  REQUIRE(hits.isEmpty());
}

TEST_CASE("HitSystem - collider with root lacking Attack is skipped") {
  // Attack を持たない root を参照するコライダーはスキップされる
  entt::registry registry;

  // Attack を持たない root エンティティ
  auto root = registry.create();

  // 攻撃コライダー（Attack 非保持の root を参照）
  auto attacker = registry.create();
  registry.emplace<WorldPos>(attacker, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      attacker,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(attacker, Attack{.damage = 5, .root = root});

  // 被弾エンティティ（重なる位置）
  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      target,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});

  const auto hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 100);
  REQUIRE(hits.isEmpty());
}

TEST_CASE("HitSystem - same Team skips the hit") {
  // 攻撃側・被弾側が同じ Team を持つ場合はヒットしない（自己ヒット・
  // 同士討ちの防止）
  entt::registry registry;

  auto attacker = registry.create();
  registry.emplace<WorldPos>(attacker, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      attacker,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(attacker, Attack{.damage = 5});
  registry.emplace<Team>(attacker, Team::Player);

  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      target,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});
  registry.emplace<Team>(target, Team::Player);

  const auto hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 100);
  REQUIRE(hits.isEmpty());
}

TEST_CASE("HitSystem - different Team allows the hit") {
  entt::registry registry;

  auto attacker = registry.create();
  registry.emplace<WorldPos>(attacker, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      attacker,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(attacker, Attack{.damage = 5});
  registry.emplace<Team>(attacker, Team::Player);

  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      target,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});
  registry.emplace<Team>(target, Team::Enemy);

  const auto hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 95);
  REQUIRE(hits.size() == 1);
}

TEST_CASE(
    "HitSystem - a side lacking Team does not participate in the Team check "
    "and still hits"
) {
  // 片方でも Team を持たなければ、陣営の判定に参加せず従来どおり当たる
  entt::registry registry;

  auto attacker = registry.create();
  registry.emplace<WorldPos>(attacker, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      attacker,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(attacker, Attack{.damage = 5});
  registry.emplace<Team>(attacker, Team::Player);

  // target は Team 非保持
  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      target,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});

  const auto hits = HitSystem::Update(registry);
  REQUIRE(registry.get<Hp>(target).current == 95);
  REQUIRE(hits.size() == 1);
}

TEST_CASE(
    "HitSystem - HitEvent resolves attackerOwner from the Hierarchy parent "
    "and copies hitstopSec/reaction"
) {
  // ヒットボックス（子）が Hierarchy 親を持つ場合、attackerOwner
  // はその親になり、hitstopSec/reaction は Attack の値がそのまま写される
  entt::registry registry;

  // 攻撃側本体（ヒットボックスの親、Collider は持たない）
  auto owner = registry.create();
  registry.emplace<WorldPos>(owner, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});

  // ヒットボックス（owner の子、Attack・Collider を保持）
  auto hitbox = registry.create();
  Hierarchy::Attach(registry, owner, hitbox);
  registry.emplace<WorldPos>(hitbox, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      hitbox,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Attack>(
      hitbox,
      Attack{.damage = 5, .hitstopSec = 0.08, .reaction = ReactionLevel::Blow}
  );

  auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 5.0, .h = 0.0, .d = 0.0});
  registry.emplace<Collider>(
      target,
      Collider{
          .segmentStart = {0.0, 0.0, 0.0},
          .segmentEnd = {0.0, 0.0, 0.0},
          .radius = 10.0
      }
  );
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});

  const auto hits = HitSystem::Update(registry);
  REQUIRE(hits.size() == 1);
  REQUIRE(hits[0].attackerOwner == owner);
  REQUIRE(hits[0].target == target);
  REQUIRE(hits[0].hitstopSec == Approx(0.08));
  REQUIRE(hits[0].reaction == ReactionLevel::Blow);
}

#endif
