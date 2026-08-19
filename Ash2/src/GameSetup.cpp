#include <Siv3D.hpp>

#include "GameSetup.hpp"

#include "Asset.hpp"
#include "Config/AnimationData.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "Config/ScenarioData.hpp"
#include "Debug.hpp"
#include "Phase/PhaseLoaders.hpp"
#include "System/HierarchySystem.hpp"
#include "System/NameLookup.hpp"

namespace {

/// @brief アニメーション設定 TOML を全件読み込む
/// @return 失敗時は toml のパスを前置したメッセージ
[[nodiscard]] std::expected<AnimationDataRegistry, String> LoadAnimations() {
  auto list = GetAssetList();
  if (!list) {
    return std::unexpected{std::move(list).error()};
  }
  AnimationDataRegistry animReg;
  for (const auto& path : *list) {
    if (FileSystem::Extension(path) != U"toml") continue;
    if (!path.starts_with(U"assets/config/animation/")) continue;
    auto data = AnimationData::FromToml(TOMLReader{AssetPath(path)});
    if (!data) {
      return std::unexpected{path + U": " + std::move(data).error()};
    }
    animReg[FileSystem::BaseName(path)] = *std::move(data);
  }
  return animReg;
}

}  // namespace

std::expected<void, String> InitializeRegistry(entt::registry& registry) {
  registry.ctx().emplace<NameLookup>();
  NameLookupSystem::Connect(registry);
  HierarchySystem::Connect(registry);

  const TOMLReader playerToml(AssetPath(U"assets/config/player.toml"));
  auto player = PlayerConfig::FromToml(playerToml);
  if (!player) {
    return std::unexpected{std::move(player).error()};
  }
  registry.ctx().emplace<PlayerConfig>(*std::move(player));

  const TOMLReader enemyToml(AssetPath(U"assets/config/enemy.toml"));
  auto enemy = EnemyConfig::FromToml(enemyToml);
  if (!enemy) {
    return std::unexpected{std::move(enemy).error()};
  }
  registry.ctx().emplace<EnemyConfig>(*std::move(enemy));

  auto anims = LoadAnimations();
  if (!anims) {
    return std::unexpected{std::move(anims).error()};
  }
  registry.ctx().emplace<AnimationDataRegistry>(*std::move(anims));

  const TOMLReader scenarioToml(AssetPath(U"assets/config/scenario.toml"));
  auto scenario = ScenarioData::FromToml(scenarioToml, GetPhaseLoaders());
  if (!scenario) {
    return std::unexpected{std::move(scenario).error()};
  }
  registry.ctx().emplace<ScenarioData>(*std::move(scenario));

  return {};
}

void ReloadConfig(entt::registry& registry) {
  const TOMLReader playerToml(AssetPath(U"assets/config/player.toml"));
  auto player = PlayerConfig::FromToml(playerToml);
  if (!player) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + player.error());
    return;
  }

  const TOMLReader enemyToml(AssetPath(U"assets/config/enemy.toml"));
  auto enemy = EnemyConfig::FromToml(enemyToml);
  if (!enemy) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + enemy.error());
    return;
  }

  auto anims = LoadAnimations();
  if (!anims) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + anims.error());
    return;
  }

  registry.ctx().get<PlayerConfig>() = *std::move(player);
  registry.ctx().get<EnemyConfig>() = *std::move(enemy);
  registry.ctx().get<AnimationDataRegistry>() = *std::move(anims);
}
