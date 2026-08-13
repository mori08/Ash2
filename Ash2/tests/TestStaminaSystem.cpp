#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Component/Motion.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/Stamina.hpp"
#include "Config/PlayerConfig.hpp"
#include "System/StaminaSystem.hpp"

namespace {

/// @brief スタミナ設定だけを載せた registry.ctx() を用意する
void SetupContext(
    entt::registry& registry, double recoveryDelay, double recoveryRate
) {
  registry.ctx().emplace<PlayerConfig>(PlayerConfig{
      .stamina = {.recoveryDelay = recoveryDelay, .recoveryRate = recoveryRate},
  });
}

/// @brief Neutral 状態のプレイヤーを生成する
entt::entity MakePlayer(entt::registry& registry, int32 max, int32 current) {
  const auto player = registry.create();
  registry.emplace<Player>(player);
  registry.emplace<Stamina>(player, Stamina{.max = max, .current = current});
  registry.emplace<Motion>(player, PlayerMotion::Neutral{});
  return player;
}

}  // namespace

TEST_CASE("StaminaSystem - does not recover before the delay elapses") {
  entt::registry registry;
  SetupContext(registry, /*recoveryDelay=*/0.5, /*recoveryRate=*/1.0);
  const auto player = MakePlayer(registry, 100, 50);

  StaminaSystem::Update(registry, 0.2);

  REQUIRE(registry.get<Stamina>(player).current == 50);
}

TEST_CASE("StaminaSystem - recovers once the delay has elapsed") {
  // 不足分（50）に recoveryRate * dt を掛けた量が回復する
  entt::registry registry;
  SetupContext(registry, /*recoveryDelay=*/0.5, /*recoveryRate=*/1.0);
  const auto player = MakePlayer(registry, 100, 50);

  StaminaSystem::Update(registry, 0.5);

  REQUIRE(registry.get<Stamina>(player).current == 75);
}

TEST_CASE("StaminaSystem - a non-Neutral motion restarts the delay") {
  // 行動中は回復せず、Neutral に戻ってから改めて待機時間を数え直す
  entt::registry registry;
  SetupContext(registry, /*recoveryDelay=*/0.5, /*recoveryRate=*/1.0);
  const auto player = MakePlayer(registry, 100, 50);
  registry.replace<Motion>(player, PlayerMotion::MeleeFinisher{});

  StaminaSystem::Update(registry, 1.0);
  REQUIRE(registry.get<Stamina>(player).current == 50);

  registry.replace<Motion>(player, PlayerMotion::Neutral{});
  StaminaSystem::Update(registry, 0.2);

  // 行動前の経過時間は持ち越さないため、まだ回復しない
  REQUIRE(registry.get<Stamina>(player).current == 50);
}

TEST_CASE("StaminaSystem - fills up even when each frame gains less than one") {
  // 1フレームの回復量が 1 未満でも、端数を積み立てるため必ず満タンに届く。
  // 端数を切り捨てる実装ではここで永久に回復しなくなる
  entt::registry registry;
  SetupContext(registry, /*recoveryDelay=*/0.0, /*recoveryRate=*/0.5);
  const auto player = MakePlayer(registry, 100, 1);

  for (int32 i = 0; i < 1000; ++i) {
    StaminaSystem::Update(registry, 0.016);
  }

  REQUIRE(registry.get<Stamina>(player).current == 100);
}

TEST_CASE("StaminaSystem - never exceeds the maximum") {
  entt::registry registry;
  SetupContext(registry, /*recoveryDelay=*/0.0, /*recoveryRate=*/1.0);
  const auto player = MakePlayer(registry, 100, 100);

  StaminaSystem::Update(registry, 1.0);

  REQUIRE(registry.get<Stamina>(player).current == 100);
}

#endif
