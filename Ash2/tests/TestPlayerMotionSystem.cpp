#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Motion.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/AnimationData.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/MotionSystem.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace {

/// @brief テスト用の registry.ctx() セットアップ（PlayerConfig +
/// AnimationDataRegistry）
void SetupContext(entt::registry& registry) {
  registry.ctx().emplace<PlayerConfig>(PlayerConfig{
      .speed = 100.0,
      .jumpSpeed = 300.0,
      .gravity = 800.0,
      .melee = {.capMidH = 40.0, .reach = 50.0, .radius = 20.0, .damage = 10},
      .ranged = {.reach = 100.0,
                 .radius = 5.0,
                 .damage = 5,
                 .bulletSpeed = 300.0,
                 .spawnHeight = 40.0},
  });

  AnimationDataRegistry animReg;
  AnimationData playerData{
      .textureKey = U"player",
      .size = {32, 32},
      .drawOffset = {0, 0},
  };
  playerData.clips[U"idle"] = AnimationClip{.row = 0, .count = 4, .speed = 4.0};
  playerData.clips[U"move"] = AnimationClip{.row = 1, .count = 4, .speed = 8.0};
  playerData.clips[U"jump"] = AnimationClip{.row = 2, .count = 1, .speed = 1.0};
  playerData.clips[U"attack"] =
      AnimationClip{.row = 3, .count = 6, .speed = 12.0};
  playerData.clips[U"ranged_attack"] =
      AnimationClip{.row = 4, .count = 4, .speed = 8.0};
  animReg[U"player"] = playerData;
  registry.ctx().emplace<AnimationDataRegistry>(std::move(animReg));
}

/// @brief テスト用のプレイヤーエンティティを生成する（地上・Neutral 付き）
entt::entity MakePlayer(entt::registry& registry) {
  const auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<Velocity>(player);
  registry.emplace<SpriteAnimation>(
      player, SpriteAnimation{.dataKey = U"player", .currentClip = U"idle"});
  registry.emplace<Motion>(player, PlayerMotion::Neutral{});
  return player;
}

}  // namespace

TEST_CASE(
    "PlayerMotionSystem - melee attack input transitions Neutral to Melee") {
  // 近接攻撃入力で Neutral から Melee へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.attackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));

  const auto& melee = std::get<PlayerMotion::Melee>(motion);
  REQUIRE(melee.timer == Approx(6.0 / 12.0));
  REQUIRE(melee.hitboxEntity != entt::entity{entt::null});
  REQUIRE(registry.valid(melee.hitboxEntity));

  REQUIRE(registry.get<SpriteAnimation>(player).currentClip == U"attack");
}

TEST_CASE(
    "PlayerMotionSystem - ranged attack input transitions Neutral to "
    "Ranged") {
  // 遠距離攻撃入力で Neutral から Ranged へ遷移し、弾エンティティが生成される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.rangedAttackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Ranged>(motion));

  const auto& ranged = std::get<PlayerMotion::Ranged>(motion);
  REQUIRE(ranged.timer == Approx(4.0 / 8.0));

  REQUIRE(registry.get<SpriteAnimation>(player).currentClip ==
          U"ranged_attack");

  // 弾エンティティが生成されている
  const auto bulletView = registry.view<Projectile>();
  REQUIRE(bulletView.size() == 1);
}

TEST_CASE("PlayerMotionSystem - no attack input keeps Neutral") {
  // 攻撃入力がない場合は Neutral のまま
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const FrameData frameData{};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - airborne player ignores attack input") {
  // 空中にいる場合は攻撃入力を無視する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;

  FrameData frameData{};
  frameData.input.attackDown = true;

  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - jump input immediately switches to jump clip") {
  // ジャンプ入力と同フレームで jump クリップに切り替わる（1フレーム遅延の修正）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.jumpDown = true;

  MotionSystem::Update(registry, frameData);

  REQUIRE(registry.get<Velocity>(player).h == Approx(300.0));
  REQUIRE(registry.get<SpriteAnimation>(player).currentClip == U"jump");
}

TEST_CASE("PlayerMotionSystem - timer expiry transitions Melee to Neutral") {
  // timer <= 0 になったら Melee から Neutral へ遷移し、ヒットボックスを破棄する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  // ヒットボックスエンティティ（Melee.hitboxEntity 用のダミー）
  const auto hitbox = registry.create();
  registry.replace<Motion>(
      player, PlayerMotion::Melee{.timer = 0.1, .hitboxEntity = hitbox});

  const FrameData frameData{.dt = 0.5};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
  REQUIRE_FALSE(registry.valid(hitbox));
}

TEST_CASE("PlayerMotionSystem - timer expiry transitions Ranged to Neutral") {
  // timer <= 0 になったら Ranged から Neutral へ遷移する（hitbox を持たない）
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(player, PlayerMotion::Ranged{.timer = 0.1});

  const FrameData frameData{.dt = 0.5};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Neutral>(motion));
}

TEST_CASE("PlayerMotionSystem - Melee timer decreases but stays in Melee") {
  // タイマーが残っている間は Melee のまま、timer が減算される
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee{.timer = 1.0, .hitboxEntity = entt::null});

  const FrameData frameData{.dt = 0.5};
  MotionSystem::Update(registry, frameData);

  const auto& motion = registry.get<Motion>(player);
  REQUIRE(std::holds_alternative<PlayerMotion::Melee>(motion));
  REQUIRE(std::get<PlayerMotion::Melee>(motion).timer == Approx(0.5));
}

TEST_CASE("PlayerMotionSystem - Melee stops horizontal movement") {
  // Melee 中は移動入力があっても横移動が止まる
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  registry.replace<Motion>(
      player, PlayerMotion::Melee{.timer = 1.0, .hitboxEntity = entt::null});

  FrameData frameData{.dt = 0.1};
  frameData.input.moveAxis = Vec2{1.0, 0.0};
  MotionSystem::Update(registry, frameData);

  const auto& vel = registry.get<Velocity>(player);
  REQUIRE(vel.w == Approx(0.0));
  REQUIRE(vel.d == Approx(0.0));
}

#endif
