#include <Siv3D.hpp>

#include "GameSetup.hpp"

#include "Asset.hpp"
#include "Config/ArenaConfig.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/InputConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "Config/ScenarioData.hpp"
#include "Config/TitleConfig.hpp"
#include "Factory/StageData.hpp"
#include "Phase/PhaseLoaders.hpp"
#include "System/HierarchySystem.hpp"
#include "System/NameLookup.hpp"
#include "UiFonts.hpp"
#include "WindowPreset.hpp"

namespace {
/// ゲームが描画に使う論理座標の大きさ（px）
///
/// ウィンドウサイズとは別の概念のため、WindowPreset と値を共有しない。
constexpr Size kSceneSize{1280, 720};
}  // namespace

AnimationDataRegistry LoadAnimations() {
  AnimationDataRegistry animReg;
  for (const auto& path : GetAssetList()) {
    if (FileSystem::Extension(path) != U"toml") continue;
    if (!path.starts_with(U"assets/config/animation/")) continue;
    const auto toml = OpenToml(path);
    auto data = AnimationData::FromToml(toml);
    if (!TextureAsset::IsRegistered(data.textureKey)) {
      throw Error{U"アニメーションのテクスチャが未登録"};
    }
    if (!TextureAsset::Load(data.textureKey)) {
      throw Error{U"アニメーションのテクスチャを読み込めない"};
    }
    animReg[FileSystem::BaseName(path)] = std::move(data);
  }
  return animReg;
}

void InitializeEngine() {
  RegisterAssets();
  UiFonts::Register();
  Scene::SetResizeMode(ResizeMode::Keep);
  Scene::Resize(kSceneSize);
  Scene::SetTextureFilter(TextureFilter::Linear);
  ApplyWindowPreset(kDefaultWindowPreset);
}

void InitializeRegistry(entt::registry& registry) {
  registry.ctx().emplace<NameLookup>();
  NameLookupSystem::Connect(registry);
  HierarchySystem::Connect(registry);

  registry.ctx().emplace<PlayerConfig>(
      PlayerConfig::FromToml(OpenToml(U"assets/config/player.toml"))
  );
  registry.ctx().emplace<EnemyConfig>(
      EnemyConfig::FromToml(OpenToml(U"assets/config/enemy.toml"))
  );
  registry.ctx().emplace<ArenaConfig>(
      ArenaConfig::FromToml(OpenToml(U"assets/config/arena.toml"))
  );
  registry.ctx().emplace<TitleConfig>(
      TitleConfig::FromToml(OpenToml(U"assets/config/title.toml"))
  );
  registry.ctx().emplace<InputConfig>(
      InputConfig::FromToml(OpenToml(U"assets/config/input.toml"))
  );
  registry.ctx().emplace<StageData>(
      StageData::FromToml(OpenToml(U"assets/config/stage.toml"))
  );
  registry.ctx().emplace<AnimationDataRegistry>(LoadAnimations());

  auto scenario = ScenarioData::FromToml(
      OpenToml(U"assets/config/scenario.toml"), GetPhaseLoaders()
  );
  if (!scenario.sections.contains(String{kInitSectionName})) {
    throw Error{U"シナリオに init セクションがない"};
  }
  registry.ctx().emplace<ScenarioData>(std::move(scenario));
}
