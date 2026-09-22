#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Attack.hpp"
#include "Component/Dead.hpp"
#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Player.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "FrameData.hpp"
#include "System/EnemySystem.hpp"
#include "System/MotionSystem.hpp"

namespace {

/// @brief テスト用の registry.ctx() セットアップ（EnemyConfig）
void SetupContext(entt::registry& registry) {
  registry.ctx().emplace<EnemyConfig>(EnemyConfig{
      .maxHp = 100,
      .capsuleRadius = 24.0,
      .capsuleHeight = 56.0,
      .spawnW = 150.0,
      .staggerSec = 0.15,
      .repelSpeed = 250.0,
      .repelSec = 0.20,
      .blowSpeedW = 300.0,
      .blowSpeedH = 300.0,
      .knockbackSec = 1.00,
      .defeatedSec = 0.50,
      .respawnSec = 1.00,
      .moveSpeed = 90.0,
      .aggroRange = 400.0,
      .leapRange = 180.0,
      .windupSec = 0.50,
      .leapSpeedW = 260.0,
      .leapSpeedH = 350.0,
      .landingSec = 0.60,
      .attackDamage = 10,
      .attackHitstopSec = 0.05,
      .attackReaction = ReactionLevel::Stagger,
  });
}

/// @brief テスト用の敵エンティティを生成する（WorldPos + Velocity +
/// SpriteAnimation + EnemyMotion::Variant）
entt::entity MakeEnemy(
    entt::registry& registry, const EnemyMotion::Variant& motion
) {
  const auto enemy = registry.create();
  registry.emplace<WorldPos>(enemy, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Velocity>(enemy);
  registry.emplace<SpriteAnimation>(
      enemy, SpriteAnimation{.dataKey = U"enemy", .currentClip = U"idle"}
  );
  registry.emplace<EnemyMotion::Variant>(enemy, motion);
  return enemy;
}

/// @brief テスト用のプレイヤーエンティティを生成する（索敵対象）
entt::entity MakePlayer(entt::registry& registry, WorldPos pos) {
  const auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player, pos);
  return player;
}

}  // namespace

TEST_CASE("EnemyMotionSystem - Idle stays Idle when no player is nearby") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Idle{});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE(
    "EnemyMotionSystem - Idle stays Idle when the player is outside "
    "aggro range"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Idle{});
  MakePlayer(registry, WorldPos{.w = 500.0});  // aggroRange 400.0 の外

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE(
    "EnemyMotionSystem - Idle does not transition to Chase while airborne"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Idle{});
  registry.get<WorldPos>(enemy).h = 50.0;
  MakePlayer(registry, WorldPos{.w = 100.0});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE(
    "EnemyMotionSystem - Idle transitions to Chase when the player is "
    "within aggro range and grounded"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Idle{});
  MakePlayer(registry, WorldPos{.w = 100.0});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Chase>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(registry.get<SpriteAnimation>(enemy).currentClip == U"move");
  REQUIRE(registry.get<SpriteAnimation>(enemy).facingRight);
}

TEST_CASE(
    "EnemyMotionSystem - Idle does not transition to Chase when the player "
    "is Dead"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Idle{});
  const auto player = MakePlayer(registry, WorldPos{.w = 100.0});
  registry.emplace<Dead>(player);

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE(
    "EnemyMotionSystem - Chase moves toward the player while outside leap "
    "range"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Chase{});
  MakePlayer(registry, WorldPos{.w = 300.0});  // leapRange 180.0 の外

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Chase>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(registry.get<Velocity>(enemy).w == Approx(90.0));
  REQUIRE(registry.get<Velocity>(enemy).d == Approx(0.0));
}

TEST_CASE(
    "EnemyMotionSystem - Chase stops and transitions to Windup within leap "
    "range"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Chase{});
  registry.get<Velocity>(enemy).w = 90.0;
  MakePlayer(registry, WorldPos{.w = 100.0});  // leapRange 180.0 の内

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Windup>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(
      std::get<EnemyMotion::Windup>(registry.get<EnemyMotion::Variant>(enemy))
          .remaining == Approx(0.50)
  );
  REQUIRE(registry.get<Velocity>(enemy).w == Approx(0.0));
  REQUIRE(registry.get<SpriteAnimation>(enemy).currentClip == U"windup");
}

TEST_CASE(
    "EnemyMotionSystem - Chase returns to Idle and zeroes Velocity when the "
    "player is Dead"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Chase{});
  registry.get<Velocity>(enemy).w = 90.0;
  const auto player = MakePlayer(registry, WorldPos{.w = 100.0});
  registry.emplace<Dead>(player);

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(registry.get<Velocity>(enemy).w == Approx(0.0));
  REQUIRE(registry.get<Velocity>(enemy).d == Approx(0.0));
  REQUIRE(registry.get<SpriteAnimation>(enemy).currentClip == U"idle");
}

TEST_CASE("EnemyMotionSystem - Windup keeps counting down while remaining") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Windup{.remaining = 0.5});
  MakePlayer(registry, WorldPos{.w = 100.0});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Windup>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(
      std::get<EnemyMotion::Windup>(registry.get<EnemyMotion::Variant>(enemy))
          .remaining == Approx(0.4)
  );
  REQUIRE_FALSE(registry.all_of<Attack>(enemy));
}

TEST_CASE(
    "EnemyMotionSystem - Windup transitions to Leap on expiry, launching "
    "toward the player and granting Attack"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Windup{.remaining = 0.01});
  MakePlayer(registry, WorldPos{.w = 100.0});  // 敵より右

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Leap>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(registry.get<Velocity>(enemy).w == Approx(260.0));
  REQUIRE(registry.get<Velocity>(enemy).d == Approx(0.0));
  REQUIRE(registry.get<Velocity>(enemy).h == Approx(350.0));
  REQUIRE(registry.get<SpriteAnimation>(enemy).currentClip == U"leap");

  REQUIRE(registry.all_of<Attack>(enemy));
  const auto& attack = registry.get<Attack>(enemy);
  REQUIRE(attack.damage == 10);
  REQUIRE(attack.hitstopSec == Approx(0.05));
  REQUIRE(attack.reaction == ReactionLevel::Stagger);
}

TEST_CASE("EnemyMotionSystem - Leap keeps state while airborne") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Leap{});
  registry.get<WorldPos>(enemy).h = 100.0;  // 空中
  registry.emplace<Attack>(enemy, Attack{.damage = 10});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Leap>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(registry.all_of<Attack>(enemy));
}

TEST_CASE(
    "EnemyMotionSystem - Leap transitions to Landing on touchdown, removing "
    "Attack and stopping horizontal velocity"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Leap{});
  registry.get<WorldPos>(enemy).h = 0.0;  // 接地
  registry.get<Velocity>(enemy).w = 260.0;
  registry.get<Velocity>(enemy).h = -10.0;  // 落下しきってクランプ済み
  registry.emplace<Attack>(enemy, Attack{.damage = 10});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Landing>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(
      std::get<EnemyMotion::Landing>(registry.get<EnemyMotion::Variant>(enemy))
          .remaining == Approx(0.60)
  );
  REQUIRE_FALSE(registry.all_of<Attack>(enemy));
  REQUIRE(registry.get<Velocity>(enemy).w == Approx(0.0));
  REQUIRE(registry.get<Velocity>(enemy).d == Approx(0.0));
  REQUIRE(registry.get<SpriteAnimation>(enemy).currentClip == U"landing");
}

TEST_CASE("EnemyMotionSystem - Landing keeps counting down while remaining") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Landing{.remaining = 0.5});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Landing>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(
      std::get<EnemyMotion::Landing>(registry.get<EnemyMotion::Variant>(enemy))
          .remaining == Approx(0.4)
  );
}

TEST_CASE(
    "EnemyMotionSystem - Landing transitions to Idle on expiry, setting "
    "idle clip"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Landing{.remaining = 0.01});

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(registry.get<SpriteAnimation>(enemy).currentClip == U"idle");
}

TEST_CASE("EnemyMotionSystem - Stagger keeps state while remaining") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Stagger{.remaining = 0.15});

  const FrameData frameData{.dt = 0.075};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Stagger>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE(
    "EnemyMotionSystem - Stagger transitions to Idle and sets idle clip on "
    "expiry"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Stagger{.remaining = 0.01});

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(registry.get<SpriteAnimation>(enemy).currentClip == U"idle");
}

TEST_CASE(
    "EnemyMotionSystem - Repel zeroes velocity and transitions to Idle on "
    "expiry"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Repel{.remaining = 0.01});
  registry.get<Velocity>(enemy).w = -250.0;

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(enemy).w == Approx(0.0));
  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE("EnemyMotionSystem - Repel keeps velocity while remaining") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Repel{.remaining = 0.5});
  registry.get<Velocity>(enemy).w = -250.0;

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(enemy).w == Approx(-250.0));
  REQUIRE(
      std::holds_alternative<EnemyMotion::Repel>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE(
    "EnemyMotionSystem - Knockback keeps horizontal velocity on the launch "
    "frame"
) {
  // 打ち上げ直後は接地したまま Tick に入るので、上昇中は止めてはならない
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Knockback{.remaining = 0.5});
  registry.get<WorldPos>(enemy).h = 0.0;  // 接地
  registry.get<Velocity>(enemy).w = 300.0;
  registry.get<Velocity>(enemy).h = 300.0;  // 上昇中

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(enemy).w == Approx(300.0));
}

TEST_CASE(
    "EnemyMotionSystem - Knockback zeroes horizontal velocity on landing"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Knockback{.remaining = 0.5});
  registry.get<WorldPos>(enemy).h = 0.0;  // 接地
  registry.get<Velocity>(enemy).w = 300.0;
  registry.get<Velocity>(enemy).h = 0.0;  // 落下しきってクランプ済み

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(enemy).w == Approx(0.0));
  REQUIRE(
      std::holds_alternative<EnemyMotion::Knockback>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE(
    "EnemyMotionSystem - Knockback keeps horizontal velocity while "
    "airborne"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Knockback{.remaining = 0.5});
  registry.get<WorldPos>(enemy).h = 100.0;  // 空中
  registry.get<Velocity>(enemy).w = 300.0;

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(enemy).w == Approx(300.0));
}

TEST_CASE("EnemyMotionSystem - Knockback transitions to Idle on expiry") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Knockback{.remaining = 0.01});

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
}

TEST_CASE("EnemyMotionSystem - Defeated fades DrawColor::color.a") {
  // 残り時間比（remaining / defeatedSec）で DrawColor::color.a
  // をフェードさせる（Drawable の形状は問わない）
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Defeated{.remaining = 0.25});
  registry.emplace<Drawable>(enemy, RectDrawable{.size = {60.0, 80.0}});

  const FrameData frameData{.dt = 0.0};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<DrawColor>(enemy).color.a == Approx(0.5));
}

TEST_CASE(
    "EnemyMotionSystem - Defeated emplaces DrawColor even without "
    "Drawable"
) {
  // Drawable を持たない敵でも DrawColor が付与される
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Defeated{.remaining = 0.50});

  const FrameData frameData{.dt = 0.0};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.all_of<DrawColor>(enemy));
  REQUIRE(registry.get<DrawColor>(enemy).color.a == Approx(1.0));
}

TEST_CASE("EnemySystem - destroys entity when Defeated has expired") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Defeated{.remaining = 0.0});

  EnemySystem::Update(registry);

  REQUIRE_FALSE(registry.valid(enemy));
}

TEST_CASE(
    "EnemySystem - keeps entity while Defeated still has remaining time"
) {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Defeated{.remaining = 0.5});

  EnemySystem::Update(registry);

  REQUIRE(registry.valid(enemy));
}

TEST_CASE("EnemySystem - leaves non-Defeated entities untouched") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Idle{});

  EnemySystem::Update(registry);

  REQUIRE(registry.valid(enemy));
}

TEST_CASE(
    "EnemyMotionSystem - Hitstop freezes Stagger remaining and keeps the "
    "stagger clip"
) {
  // dt = 0 で Tick されるため、残り時間も idle への遷移も進まない
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Stagger{.remaining = 0.15});
  registry.get<SpriteAnimation>(enemy).currentClip = U"stagger";
  registry.emplace<Hitstop>(enemy, Hitstop{.remaining = 0.1});

  const FrameData frameData{.dt = 0.075};
  MotionSystem::Update(registry, frameData);

  REQUIRE(
      std::holds_alternative<EnemyMotion::Stagger>(
          registry.get<EnemyMotion::Variant>(enemy)
      )
  );
  REQUIRE(
      std::get<EnemyMotion::Stagger>(registry.get<EnemyMotion::Variant>(enemy))
          .remaining == Approx(0.15)
  );
  REQUIRE(registry.get<SpriteAnimation>(enemy).currentClip == U"stagger");
}

TEST_CASE(
    "EnemyMotionSystem - Hitstop keeps Knockback vel.w on the launch frame"
) {
  // blowSpeedH（テスト設定値 300.0）が正である限り、着地判定と組み合わさる
  // vel.h <= 0 ガードには入らないため vel.w は消えない。blowSpeedH
  // を 0 にする将来の調整で、このテストが失敗して気付けるようにする
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Knockback{.remaining = 0.5});
  registry.get<WorldPos>(enemy).h = 0.0;  // 接地
  registry.get<Velocity>(enemy).w = 300.0;
  registry.get<Velocity>(enemy).h = 300.0;  // 上昇中（打ち上げ直後）
  registry.emplace<Hitstop>(enemy, Hitstop{.remaining = 0.1});

  const FrameData frameData{.dt = 0.1};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(enemy).w == Approx(300.0));
}

TEST_CASE(
    "EnemyMotionSystem - Hitstop prevents Defeated from being destroyed by "
    "EnemySystem"
) {
  // dt = 0 で remaining が凍結されるため、停止中は EnemySystem
  // に破棄されない
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Defeated{.remaining = 0.01});
  registry.emplace<Hitstop>(enemy, Hitstop{.remaining = 0.1});

  const FrameData frameData{.dt = 1.0};
  MotionSystem::Update(registry, frameData);
  EnemySystem::Update(registry);

  REQUIRE(registry.valid(enemy));
}

#endif
