#include <Siv3D.hpp>

#include "GameSetup.hpp"

#include "Asset.hpp"
#include "Config/ArenaConfig.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "Config/ScenarioData.hpp"
#include "Debug.hpp"
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

std::expected<AnimationDataRegistry, String> LoadAnimations() {
  auto list = GetAssetList();
  if (!list) {
    return std::unexpected{std::move(list).error()};
  }
  AnimationDataRegistry animReg;
  for (const auto& path : *list) {
    if (FileSystem::Extension(path) != U"toml") continue;
    if (!path.starts_with(U"assets/config/animation/")) continue;
    auto toml = OpenToml(path);
    if (!toml) {
      return std::unexpected{std::move(toml).error()};
    }
    auto data = AnimationData::FromToml(*toml);
    if (!data) {
      return std::unexpected{path + U": " + std::move(data).error()};
    }
    if (!TextureAsset::IsRegistered(data->textureKey)) {
      return std::unexpected{
          U"LoadAnimations: {} の texture '{}' が未登録です"_fmt(
              path, data->textureKey
          )
      };
    }
    if (!TextureAsset::Load(data->textureKey)) {
      return std::unexpected{
          U"LoadAnimations: {} の texture '{}' を読み込めません"_fmt(
              path, data->textureKey
          )
      };
    }
    animReg[FileSystem::BaseName(path)] = *std::move(data);
  }
  return animReg;
}

std::expected<void, String> InitializeEngine() {
  if (auto result = RegisterAssets(); !result) {
    return std::unexpected{std::move(result).error()};
  }
  if (auto result = UiFonts::Register(); !result) {
    return std::unexpected{std::move(result).error()};
  }
  Scene::SetResizeMode(ResizeMode::Keep);
  Scene::Resize(kSceneSize);
  Scene::SetTextureFilter(TextureFilter::Linear);
  // Keep によりどの大きさでも描画は成り立つため、失敗しても起動を止めない
  if (auto result = ApplyWindowPreset(kDefaultWindowPreset); !result) {
    APP_LOG(std::move(result).error());
  }
  return {};
}

std::expected<void, String> InitializeRegistry(entt::registry& registry) {
  registry.ctx().emplace<NameLookup>();
  NameLookupSystem::Connect(registry);
  HierarchySystem::Connect(registry);

  auto playerToml = OpenToml(U"assets/config/player.toml");
  if (!playerToml) {
    return std::unexpected{std::move(playerToml).error()};
  }
  auto player = PlayerConfig::FromToml(*playerToml);
  if (!player) {
    return std::unexpected{std::move(player).error()};
  }
  registry.ctx().emplace<PlayerConfig>(*std::move(player));

  auto enemyToml = OpenToml(U"assets/config/enemy.toml");
  if (!enemyToml) {
    return std::unexpected{std::move(enemyToml).error()};
  }
  auto enemy = EnemyConfig::FromToml(*enemyToml);
  if (!enemy) {
    return std::unexpected{std::move(enemy).error()};
  }
  registry.ctx().emplace<EnemyConfig>(*std::move(enemy));

  auto arenaToml = OpenToml(U"assets/config/arena.toml");
  if (!arenaToml) {
    return std::unexpected{std::move(arenaToml).error()};
  }
  auto arena = ArenaConfig::FromToml(*arenaToml);
  if (!arena) {
    return std::unexpected{std::move(arena).error()};
  }
  registry.ctx().emplace<ArenaConfig>(*std::move(arena));

  auto stageToml = OpenToml(U"assets/config/stage.toml");
  if (!stageToml) {
    return std::unexpected{std::move(stageToml).error()};
  }
  auto stage = StageData::FromToml(*stageToml);
  if (!stage) {
    return std::unexpected{std::move(stage).error()};
  }
  registry.ctx().emplace<StageData>(*std::move(stage));

  auto anims = LoadAnimations();
  if (!anims) {
    return std::unexpected{std::move(anims).error()};
  }
  registry.ctx().emplace<AnimationDataRegistry>(*std::move(anims));

  auto scenarioToml = OpenToml(U"assets/config/scenario.toml");
  if (!scenarioToml) {
    return std::unexpected{std::move(scenarioToml).error()};
  }
  auto scenario = ScenarioData::FromToml(*scenarioToml, GetPhaseLoaders());
  if (!scenario) {
    return std::unexpected{std::move(scenario).error()};
  }
  if (!scenario->sections.contains(String{kInitSectionName})) {
    return std::unexpected{
        U"InitializeRegistry: セクション \"{}\" がありません"_fmt(
            kInitSectionName
        )
    };
  }
  registry.ctx().emplace<ScenarioData>(*std::move(scenario));

  return {};
}
