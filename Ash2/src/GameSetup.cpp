#include <Siv3D.hpp>

#include "GameSetup.hpp"

#include "Asset.hpp"
#include "Config/AnimationData.hpp"
#include "Config/ArenaConfig.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/InputConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "Config/ScenarioData.hpp"
#include "Config/TitleConfig.hpp"
#include "DebugOnly.hpp"
#include "Factory/StageData.hpp"
#include "Phase/PhaseLoaders.hpp"
#include "Phase/TitlePhase.hpp"
#include "System/HierarchySystem.hpp"
#include "System/NameLookup.hpp"
#include "UiFonts.hpp"
#include "WindowPreset.hpp"

namespace {
/// ゲームが描画に使う論理座標の大きさ（px）
///
/// ウィンドウサイズとは別の概念のため、WindowPreset と値を共有しない。
constexpr Size kSceneSize{1280, 720};

/// @brief アニメーション設定 TOML を全件読み込む
/// @throw Error TOML を開けない、またはテクスチャが未登録・読み込めないとき
[[nodiscard]] AnimationDataRegistry LoadAnimations() {
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
}  // namespace

void InitializeEngine() {
  RegisterAssets();
  UiFonts::Register();
  Scene::SetResizeMode(ResizeMode::Keep);
  Scene::Resize(kSceneSize);
  Scene::SetTextureFilter(TextureFilter::Linear);
  ApplyWindowPreset(kDefaultWindowPreset);
}

void LoadConfigs(entt::registry& registry) {
  auto& ctx = registry.ctx();
  ctx.insert_or_assign(
      PlayerConfig::FromToml(OpenToml(U"assets/config/player.toml"))
  );
  ctx.insert_or_assign(
      EnemyConfig::FromToml(OpenToml(U"assets/config/enemy.toml"))
  );
  ctx.insert_or_assign(
      ArenaConfig::FromToml(OpenToml(U"assets/config/arena.toml"))
  );
  ctx.insert_or_assign(
      TitleConfig::FromToml(OpenToml(U"assets/config/title.toml"))
  );
  ctx.insert_or_assign(
      InputConfig::FromToml(OpenToml(U"assets/config/input.toml"))
  );
  ctx.insert_or_assign(
      StageData::FromToml(OpenToml(U"assets/config/stage.toml"))
  );
  ctx.insert_or_assign(LoadAnimations());
}

void InitializeRegistry(entt::registry& registry) {
  registry.ctx().emplace<NameLookup>();
  NameLookupSystem::Connect(registry);
  HierarchySystem::Connect(registry);

  LoadConfigs(registry);

  auto scenario = ScenarioData::FromToml(
      OpenToml(U"assets/config/scenario.toml"), GetPhaseLoaders()
  );
  if (!scenario.sections.contains(String{kInitSectionName})) {
    throw Error{U"シナリオに init セクションがない"};
  }
  if (!scenario.sections.contains(String{kTitleStartSectionName})) {
    throw Error{U"シナリオに TitleStart セクションがない"};
  }
  if (const auto start = DebugOnly::StartSectionOverride();
      start && !scenario.sections.contains(*start)) {
    throw Error{U"シナリオに開始セクション " + *start + U" がない"};
  }
  registry.ctx().emplace<ScenarioData>(std::move(scenario));
}
