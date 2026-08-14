#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Enemy.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Hp.hpp"
#include "Component/Motion.hpp"
#include "Component/ReactionLevel.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "System/HitReactionSystem.hpp"
#include "System/HitSystem.hpp"

namespace {

/// @brief テスト用の registry.ctx() セットアップ（EnemyConfig）
void SetupContext(entt::registry& registry) {
  registry.ctx().emplace<EnemyConfig>(EnemyConfig{
      .maxHp = 100,
      .size = {60.0, 80.0},
      .capsuleRadius = 30.0,
      .capsuleHeight = 80.0,
      .spawnW = 150.0,
      .staggerSec = 0.15,
      .repelSpeed = 250.0,
      .repelSec = 0.20,
      .blowSpeedW = 300.0,
      .blowSpeedH = 300.0,
      .knockbackSec = 1.00,
      .defeatedSec = 0.50,
      .respawnSec = 1.00,
  });
}

/// @brief 攻撃側本体（親）とヒットボックス（子、Attack 保持）を生成し、
/// ヒットボックスエンティティを返す
entt::entity MakeAttacker(
    entt::registry& registry, double ownerW, ReactionLevel reaction,
    double hitstopSec = 0.05
) {
  const auto owner = registry.create();
  registry.emplace<WorldPos>(owner, WorldPos{.w = ownerW});

  const auto hitbox = registry.create();
  Hierarchy::Attach(registry, owner, hitbox);
  registry.emplace<Attack>(
      hitbox,
      Attack{.damage = 10, .hitstopSec = hitstopSec, .reaction = reaction}
  );
  return hitbox;
}

/// @brief テスト用の敵（被弾側）エンティティを生成する
entt::entity MakeTarget(entt::registry& registry, double targetW) {
  const auto target = registry.create();
  registry.emplace<Enemy>(target);
  registry.emplace<WorldPos>(target, WorldPos{.w = targetW});
  registry.emplace<Velocity>(target);
  registry.emplace<Collider>(target);
  registry.emplace<Hp>(target, Hp{.max = 100, .current = 100});
  registry.emplace<Motion>(target, EnemyMotion::Idle{});
  return target;
}

}  // namespace

TEST_CASE("HitReactionSystem - Stagger reaction transitions Enemy to Stagger") {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker = MakeAttacker(registry, 0.0, ReactionLevel::Stagger);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Stagger>(registry.get<Motion>(target))
  );
}

TEST_CASE(
    "HitReactionSystem - Repel reaction transitions Enemy to Repel and sets "
    "velocity"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker = MakeAttacker(registry, 0.0, ReactionLevel::Repel);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Repel>(registry.get<Motion>(target))
  );
  REQUIRE(registry.get<Velocity>(target).w == Approx(250.0));
}

TEST_CASE(
    "HitReactionSystem - Blow reaction transitions Enemy to Knockback and "
    "sets velocity"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker = MakeAttacker(registry, 0.0, ReactionLevel::Blow);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<Motion>(target)
      )
  );
  REQUIRE(registry.get<Velocity>(target).w == Approx(300.0));
  REQUIRE(registry.get<Velocity>(target).h == Approx(300.0));
}

TEST_CASE("HitReactionSystem - None reaction leaves Enemy in Idle") {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker = MakeAttacker(registry, 0.0, ReactionLevel::None);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(registry.get<Motion>(target))
  );
}

TEST_CASE(
    "HitReactionSystem - Hp depletion forces Defeated regardless of "
    "reaction"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker = MakeAttacker(registry, 0.0, ReactionLevel::Stagger);
  const auto target = MakeTarget(registry, 50.0);
  registry.get<Hp>(target).current = 0;

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Defeated>(
          registry.get<Motion>(target)
      )
  );
  REQUIRE_FALSE(registry.all_of<Collider>(target));
  REQUIRE_FALSE(registry.all_of<Hp>(target));
}

TEST_CASE(
    "HitReactionSystem - hit from an owner to the left pushes target to the "
    "right (positive vel.w)"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker =
      MakeAttacker(registry, /*ownerW=*/0.0, ReactionLevel::Repel);
  const auto target = MakeTarget(registry, /*targetW=*/50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(registry.get<Velocity>(target).w > 0.0);
}

TEST_CASE(
    "HitReactionSystem - hit from an owner to the right pushes target to "
    "the left (negative vel.w)"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker =
      MakeAttacker(registry, /*ownerW=*/100.0, ReactionLevel::Repel);
  const auto target = MakeTarget(registry, /*targetW=*/50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(registry.get<Velocity>(target).w < 0.0);
}

TEST_CASE(
    "HitReactionSystem - hitstopSec <= 0 skips hitstop but still applies "
    "reaction"
) {
  // 弾（Ranged）は hitstopSec 0 で作られるが、ひるみ・撃破自体は起きる
  entt::registry registry;
  SetupContext(registry);
  const auto attacker =
      MakeAttacker(registry, 0.0, ReactionLevel::Blow, /*hitstopSec=*/0.0);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<Motion>(target)
      )
  );
  REQUIRE_FALSE(registry.all_of<Hitstop>(target));
}

TEST_CASE(
    "HitReactionSystem - hitstopSec <= 0 still forces Defeated on Hp "
    "depletion"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker =
      MakeAttacker(registry, 0.0, ReactionLevel::None, /*hitstopSec=*/0.0);
  const auto target = MakeTarget(registry, 50.0);
  registry.get<Hp>(target).current = 0;

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Defeated>(
          registry.get<Motion>(target)
      )
  );
  REQUIRE_FALSE(registry.all_of<Hitstop>(target));
}

TEST_CASE(
    "HitReactionSystem - Stagger hit while Repel resets Velocity.w to "
    "zero"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);
  registry.replace<Motion>(target, EnemyMotion::Repel{.remaining = 0.1});
  registry.get<Velocity>(target).w = 250.0;
  const auto attacker = MakeAttacker(registry, 0.0, ReactionLevel::Stagger);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Stagger>(registry.get<Motion>(target))
  );
  REQUIRE(registry.get<Velocity>(target).w == Approx(0.0));
}

TEST_CASE(
    "HitReactionSystem - Blow hit while Stagger restores RectDrawable "
    "size"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);
  registry.replace<Motion>(target, EnemyMotion::Stagger{.remaining = 0.05});
  registry.emplace<Drawable>(target, RectDrawable{.size = {60.0, 40.0}});
  const auto attacker = MakeAttacker(registry, 0.0, ReactionLevel::Blow);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<Motion>(target)
      )
  );
  const auto& rect = std::get<RectDrawable>(registry.get<Drawable>(target));
  REQUIRE(rect.size.y == Approx(80.0));
}

TEST_CASE(
    "HitReactionSystem - None reaction while Knockback keeps Velocity "
    "untouched"
) {
  // 弾（reaction 既定の None）が当たっても、進行中の Knockback は乱さない
  entt::registry registry;
  SetupContext(registry);
  const auto target = MakeTarget(registry, 50.0);
  registry.replace<Motion>(target, EnemyMotion::Knockback{.remaining = 0.5});
  registry.get<Velocity>(target).w = 300.0;
  registry.get<Velocity>(target).h = 300.0;
  const auto attacker =
      MakeAttacker(registry, 0.0, ReactionLevel::None, /*hitstopSec=*/0.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<Motion>(target)
      )
  );
  REQUIRE(registry.get<Velocity>(target).w == Approx(300.0));
  REQUIRE(registry.get<Velocity>(target).h == Approx(300.0));
}

TEST_CASE(
    "HitReactionSystem - grants Hitstop to the attacker's owner and the "
    "target"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto attacker = MakeAttacker(
      registry, 0.0, ReactionLevel::Stagger,
      /*hitstopSec=*/0.1
  );
  const auto target = MakeTarget(registry, 50.0);
  const auto owner = registry.get<Hierarchy>(attacker).parent();

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(registry.all_of<Hitstop>(owner));
  REQUIRE(registry.all_of<Hitstop>(target));
}

TEST_CASE(
    "HitReactionSystem - overlapping Hitstop keeps the longer remaining "
    "time"
) {
  // 停止中に短いヒットが重なっても、長い方の残り時間を維持する
  entt::registry registry;
  SetupContext(registry);
  const auto longAttacker =
      MakeAttacker(registry, 0.0, ReactionLevel::Stagger, /*hitstopSec=*/0.2);
  const auto shortAttacker =
      MakeAttacker(registry, 0.0, ReactionLevel::Stagger, /*hitstopSec=*/0.05);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = longAttacker, .target = target}}
  );
  REQUIRE(registry.get<Hitstop>(target).remaining == Approx(0.2));

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = shortAttacker, .target = target}}
  );
  REQUIRE(registry.get<Hitstop>(target).remaining == Approx(0.2));
}

TEST_CASE(
    "HitReactionSystem - overlapping Hitstop extends to a longer new "
    "remaining time"
) {
  // 停止中により長いヒットが重なったら、その長い方へ更新する
  entt::registry registry;
  SetupContext(registry);
  const auto shortAttacker =
      MakeAttacker(registry, 0.0, ReactionLevel::Stagger, /*hitstopSec=*/0.05);
  const auto longAttacker =
      MakeAttacker(registry, 0.0, ReactionLevel::Stagger, /*hitstopSec=*/0.2);
  const auto target = MakeTarget(registry, 50.0);

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = shortAttacker, .target = target}}
  );
  REQUIRE(registry.get<Hitstop>(target).remaining == Approx(0.05));

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = longAttacker, .target = target}}
  );
  REQUIRE(registry.get<Hitstop>(target).remaining == Approx(0.2));
}

TEST_CASE(
    "HitReactionSystem - non-Enemy target only receives Hitstop, no "
    "reaction applied"
) {
  // Enemy を持たない被弾側（プレイヤー想定）はリアクション適用の対象外
  entt::registry registry;
  SetupContext(registry);
  const auto attacker =
      MakeAttacker(registry, 0.0, ReactionLevel::Blow, /*hitstopSec=*/0.1);

  const auto target = registry.create();
  registry.emplace<WorldPos>(target, WorldPos{.w = 50.0});

  HitReactionSystem::Apply(
      registry, {HitPair{.attacker = attacker, .target = target}}
  );

  REQUIRE(registry.all_of<Hitstop>(target));
  REQUIRE_FALSE(registry.all_of<Motion>(target));
}

#endif
