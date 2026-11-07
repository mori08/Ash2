#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hp.hpp"
#include "Component/Invincible.hpp"
#include "Component/Team.hpp"
#include "Component/WorldPos.hpp"
#include "Config/ReactionLevel.hpp"
#include "System/HitSystem.hpp"

namespace {

/// @brief 半径 10 の球（線分の長さ 0 のカプセル）
const Collider kSphere{
    .segmentStart = {0.0, 0.0, 0.0},
    .segmentEnd = {0.0, 0.0, 0.0},
    .radius = 10.0
};

/// @brief w の位置に Player 陣営の攻撃（ダメージ 5）を生成する
entt::entity MakeAttacker(entt::registry& registry, double w) {
  const auto attacker = registry.create();
  registry.emplace<WorldPos>(attacker, WorldPos{.w = w});
  registry.emplace<Collider>(attacker, kSphere);
  registry.emplace<Attack>(attacker, Attack{.damage = 5});
  registry.emplace<Team>(attacker, Team::Player);
  return attacker;
}

/// @brief w の位置に Enemy 陣営の被弾対象（HP 100）を生成する
entt::entity MakeTarget(entt::registry& registry, double w) {
  const auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = w});
  registry.emplace<Collider>(target, kSphere);
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});
  registry.emplace<Team>(target, Team::Enemy);
  return target;
}

}  // namespace

TEST_CASE("HitSystem - separated colliders do not hit") {
  // 中心間 25 は半径の和 20 を超えるため重ならない
  entt::registry registry;
  MakeAttacker(registry, 0.0);
  const auto target = MakeTarget(registry, 25.0);

  const auto hits = HitSystem::Update(registry);

  REQUIRE(hits.isEmpty());
  REQUIRE(registry.get<Hp>(target).current == 100);
}

TEST_CASE("HitSystem - an Invincible target is not hit") {
  entt::registry registry;
  MakeAttacker(registry, 0.0);
  const auto target = MakeTarget(registry, 5.0);
  registry.emplace<Invincible>(target);

  const auto hits = HitSystem::Update(registry);

  REQUIRE(hits.isEmpty());
  REQUIRE(registry.get<Hp>(target).current == 100);
}

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
  registry.emplace<Team>(attacker, Team::Player);

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
  registry.emplace<Team>(target, Team::Enemy);

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
  registry.emplace<Team>(hitbox, Team::Player);

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
  REQUIRE(hits.size() == 1);
  REQUIRE(hits[0].attackerOwner == owner);
  REQUIRE(hits[0].target == target);
  REQUIRE(hits[0].hitstopSec == Approx(0.08));
  REQUIRE(hits[0].reaction == ReactionLevel::Blow);
}

#endif
