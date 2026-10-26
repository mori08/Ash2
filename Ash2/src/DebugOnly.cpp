#include "DebugOnly.hpp"

#ifdef _DEBUG

#include <Siv3D.hpp>

#include <cstdlib>
#include <iostream>

#include "Asset.hpp"
#include "Config/ArenaConfig.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/InputConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "Config/TitleConfig.hpp"
#include "Debug.hpp"
#include "Factory/StageData.hpp"
#include "GameSetup.hpp"
#include "System/DebugDrawSystem.hpp"

#define CATCH_CONFIG_RUNNER
#include <ThirdParty/Catch2/catch.hpp>

namespace {

// Siv3D 予約キー（割り当て不可）:
//   F1 = ライセンス表示 / F12・PrintScreen = スクリーンショット
constexpr Input kConfigReloadKey = KeyF5;
constexpr Input kColliderDrawKey = KeyF2;

/// Collider のデバッグ描画を表示中か（既定は非表示）
bool colliderDrawEnabled = false;

/// @brief 設定を再読込する
/// @note Config（Player / Enemy / Arena / Title / Input / AnimationData）の
/// 読み込みに失敗すると throw するため、ゲームが終了する。
/// それ以外（TOML を開けない、StageData、アニメーションのテクスチャ）は
/// 失敗時に旧データを維持して戻る
// TODO(#368): StageData と LoadAnimations
// のテクスチャは失敗時に旧データを維持し、 Config と挙動が混在している
void ReloadConfig(entt::registry& registry) {
  auto playerToml = OpenToml(U"assets/config/player.toml");
  if (!playerToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + playerToml.error());
    return;
  }
  auto player = PlayerConfig::FromToml(*playerToml);

  auto enemyToml = OpenToml(U"assets/config/enemy.toml");
  if (!enemyToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + enemyToml.error());
    return;
  }
  auto enemy = EnemyConfig::FromToml(*enemyToml);

  auto arenaToml = OpenToml(U"assets/config/arena.toml");
  if (!arenaToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + arenaToml.error());
    return;
  }
  auto arena = ArenaConfig::FromToml(*arenaToml);

  auto titleToml = OpenToml(U"assets/config/title.toml");
  if (!titleToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + titleToml.error());
    return;
  }
  auto title = TitleConfig::FromToml(*titleToml);

  auto inputToml = OpenToml(U"assets/config/input.toml");
  if (!inputToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + inputToml.error());
    return;
  }
  auto input = InputConfig::FromToml(*inputToml);

  auto stageToml = OpenToml(U"assets/config/stage.toml");
  if (!stageToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + stageToml.error());
    return;
  }
  auto stage = StageData::FromToml(*stageToml);
  if (!stage) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + stage.error());
    return;
  }

  auto anims = LoadAnimations();
  if (!anims) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + anims.error());
    return;
  }

  registry.ctx().get<PlayerConfig>() = std::move(player);
  registry.ctx().get<EnemyConfig>() = enemy;
  registry.ctx().get<ArenaConfig>() = arena;
  registry.ctx().get<TitleConfig>() = title;
  registry.ctx().get<InputConfig>() = input;
  registry.ctx().get<StageData>() = *std::move(stage);
  registry.ctx().get<AnimationDataRegistry>() = *std::move(anims);
}

}  // namespace

namespace DebugOnly {

void RunTestsIfRequested() {
  size_t envLen = 0;
  if (getenv_s(&envLen, nullptr, 0, "ASH2_RUN_TESTS") != 0 || envLen == 0) {
    return;
  }
  AppDebug::testMode = true;
  const int32 result = Catch::Session().run();
  // std::exit() は s3d のスレッドと噛み合って終了しない
  // _Exit() は出力を流さないため、先に明示的に流す
  // TODO(#278): 終了コードを設定する手段がなく _Exit に頼っている
  std::cout.flush();
  std::_Exit(result);
}

void OpenDebugConsole() {
  Console.open();
  APP_LOG(U"=== Debug Build ===");
}

void UpdateConfigReload(entt::registry& registry) {
  if (kConfigReloadKey.down()) {
    ReloadConfig(registry);
  }
}

void DrawColliders(const entt::registry& registry) {
  if (kColliderDrawKey.down()) {
    colliderDrawEnabled = !colliderDrawEnabled;
  }
  if (colliderDrawEnabled) {
    DebugDrawSystem::DrawColliders(registry);
    DebugDrawSystem::DrawBoundary(registry);
  }
}

}  // namespace DebugOnly

#endif
