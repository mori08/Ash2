#include <Siv3D.hpp>

#include "GameSetup.hpp"

#include "Asset.hpp"
#include "Component/AnimationData.hpp"
#include "Config/PlayerConfig.hpp"
#include "Config/ScenarioData.hpp"
#include "Phase/PhaseRegistry.hpp"
#include "System/NameLookup.hpp"

namespace {

void LoadAnimations(entt::registry& registry) {
  assert(registry.ctx().find<AnimationDataRegistry>() != nullptr);
  auto& animReg = registry.ctx().get<AnimationDataRegistry>();
  animReg.clear();
  for (const auto& path : GetAssetList()) {
    if (FileSystem::Extension(path) != U"toml") continue;
    if (!path.starts_with(U"assets/config/animation/")) continue;
    animReg[FileSystem::BaseName(path)] =
        AnimationData::FromToml(TOMLReader{AssetPath(path)});
  }
}

}  // namespace

void InitializeRegistry(entt::registry& registry) {
  registry.ctx().emplace<NameLookup>();
  NameLookupSystem::Connect(registry);

  const TOMLReader playerToml(AssetPath(U"assets/config/player.toml"));
  registry.ctx().emplace<PlayerConfig>(PlayerConfig::FromToml(playerToml));

  registry.ctx().emplace<AnimationDataRegistry>();
  LoadAnimations(registry);

  const TOMLReader scenarioToml(AssetPath(U"assets/config/scenario.toml"));
  registry.ctx().emplace<ScenarioData>(ScenarioData::FromToml(scenarioToml));

  registry.ctx().emplace<PhaseRegistry>(MakeDefaultPhaseRegistry());
}

void ReloadConfig(entt::registry& registry) {
  const TOMLReader playerToml(AssetPath(U"assets/config/player.toml"));
  registry.ctx().get<PlayerConfig>() = PlayerConfig::FromToml(playerToml);
  LoadAnimations(registry);
}
