#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/AnimationData.hpp"
#include "Component/AttackState.hpp"
#include "Component/NeutralState.hpp"
#include "Component/Player.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"
#include "System/NeutralStateSystem.hpp"

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
  playerData.clips[U"attack"] =
      AnimationClip{.row = 0, .count = 6, .speed = 12.0};
  playerData.clips[U"ranged_attack"] =
      AnimationClip{.row = 1, .count = 4, .speed = 8.0};
  animReg[U"player"] = playerData;
  registry.ctx().emplace<AnimationDataRegistry>(std::move(animReg));
}

/// @brief テスト用のプレイヤーエンティティを生成する（地上・NeutralState 付き）
entt::entity MakePlayer(entt::registry& registry) {
  const auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<WorldPos>(player, WorldPos{.w = 0.0, .h = 0.0, .d = 0.0});
  registry.emplace<SpriteAnimation>(
      player, SpriteAnimation{.dataKey = U"player", .currentClip = U"idle"});
  registry.emplace<NeutralState>(player);
  return player;
}

}  // namespace

TEST_CASE(
    "NeutralStateSystem - melee attack input transitions Neutral to Attack") {
  // 近接攻撃入力で NeutralState から AttackState へ遷移する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.attackDown = true;

  NeutralStateSystem::Update(registry, frameData);

  REQUIRE_FALSE(registry.all_of<NeutralState>(player));
  REQUIRE(registry.all_of<AttackState>(player));

  const auto& attackState = registry.get<AttackState>(player);
  REQUIRE(attackState.clip == U"attack");
  REQUIRE(attackState.timer == Approx(6.0 / 12.0));
  REQUIRE(attackState.entity != entt::entity{entt::null});
  REQUIRE(registry.valid(attackState.entity));
}

TEST_CASE(
    "NeutralStateSystem - ranged attack input transitions Neutral to "
    "Attack") {
  // 遠距離攻撃入力で NeutralState から AttackState へ遷移し、entity は
  // entt::null のまま
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  FrameData frameData{};
  frameData.input.rangedAttackDown = true;

  NeutralStateSystem::Update(registry, frameData);

  REQUIRE_FALSE(registry.all_of<NeutralState>(player));
  REQUIRE(registry.all_of<AttackState>(player));

  const auto& attackState = registry.get<AttackState>(player);
  REQUIRE(attackState.clip == U"ranged_attack");
  REQUIRE(attackState.timer == Approx(4.0 / 8.0));
  REQUIRE(attackState.entity == entt::entity{entt::null});

  // 弾エンティティが生成されている
  const auto bulletView = registry.view<Projectile>();
  REQUIRE(bulletView.size() == 1);
}

TEST_CASE("NeutralStateSystem - no attack input keeps NeutralState") {
  // 攻撃入力がない場合は NeutralState のまま
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);

  const FrameData frameData{};
  NeutralStateSystem::Update(registry, frameData);

  REQUIRE(registry.all_of<NeutralState>(player));
  REQUIRE_FALSE(registry.all_of<AttackState>(player));
}

TEST_CASE("NeutralStateSystem - airborne player ignores attack input") {
  // 空中にいる場合は攻撃入力を無視する
  entt::registry registry;
  SetupContext(registry);
  const auto player = MakePlayer(registry);
  registry.get<WorldPos>(player).h = 50.0;

  FrameData frameData{};
  frameData.input.attackDown = true;

  NeutralStateSystem::Update(registry, frameData);

  REQUIRE(registry.all_of<NeutralState>(player));
  REQUIRE_FALSE(registry.all_of<AttackState>(player));
}

#endif
