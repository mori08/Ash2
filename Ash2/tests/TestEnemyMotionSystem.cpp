#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Drawable.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Motion.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/EnemySystem.hpp"
#include "System/MotionSystem.hpp"

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

/// @brief テスト用の敵エンティティを生成する（WorldPos + Velocity + Motion）
entt::entity MakeEnemy(entt::registry& registry, const Motion& motion) {
  const auto enemy = registry.create();
  registry.emplace<WorldPos>(enemy, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Velocity>(enemy);
  registry.emplace<Motion>(enemy, motion);
  return enemy;
}

}  // namespace

TEST_CASE("EnemyMotionSystem - Stagger shrinks RectDrawable vertically") {
  // 縦縮み量は progress に応じて変化する（残り時間が duration の半分の地点）
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Stagger{.remaining = 0.15});
  registry.emplace<Drawable>(
      enemy, RectDrawable{.size = {60.0, 80.0}, .color = ColorF{1.0}});

  const FrameData frameData{.dt = 0.075};
  MotionSystem::Update(registry, frameData);

  const auto& rect = std::get<RectDrawable>(registry.get<Drawable>(enemy));
  REQUIRE(rect.size.y < 80.0);
  REQUIRE(std::holds_alternative<EnemyMotion::Stagger>(
      registry.get<Motion>(enemy)));
}

TEST_CASE(
    "EnemyMotionSystem - Stagger restores original size and transitions to "
    "Idle on expiry") {
  // 満了時（remaining <= 0）は EnemyConfig::size を代入して原寸に戻し、
  // Idle へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto enemy =
      MakeEnemy(registry, EnemyMotion::Stagger{.remaining = 0.01});
  registry.emplace<Drawable>(
      enemy, RectDrawable{.size = {60.0, 60.0}, .color = ColorF{1.0}});

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  const auto& rect = std::get<RectDrawable>(registry.get<Drawable>(enemy));
  REQUIRE(rect.size.x == Approx(60.0));
  REQUIRE(rect.size.y == Approx(80.0));
  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(registry.get<Motion>(enemy)));
}

TEST_CASE(
    "EnemyMotionSystem - Repel zeroes velocity and transitions to Idle on "
    "expiry") {
  entt::registry registry;
  SetupContext(registry);
  const auto enemy = MakeEnemy(registry, EnemyMotion::Repel{.remaining = 0.01});
  registry.get<Velocity>(enemy).w = -250.0;

  const FrameData frameData{.dt = 0.02};
  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(enemy).w == Approx(0.0));
  REQUIRE(
      std::holds_alternative<EnemyMotion::Idle>(registry.get<Motion>(enemy)));
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
      std::holds_alternative<EnemyMotion::Repel>(registry.get<Motion>(enemy)));
}

TEST_CASE(
    "EnemyMotionSystem - Knockback keeps horizontal velocity on the launch "
    "frame") {
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
    "EnemyMotionSystem - Knockback zeroes horizontal velocity on landing") {
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
  REQUIRE(std::holds_alternative<EnemyMotion::Knockback>(
      registry.get<Motion>(enemy)));
}

TEST_CASE(
    "EnemyMotionSystem - Knockback keeps horizontal velocity while "
    "airborne") {
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
      std::holds_alternative<EnemyMotion::Idle>(registry.get<Motion>(enemy)));
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
    "EnemySystem - keeps entity while Defeated still has remaining time") {
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

#endif
