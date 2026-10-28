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
/// @note 読み込みに失敗すると throw し、ゲームが終了する
void ReloadConfig(entt::registry& registry) {
  auto player = PlayerConfig::FromToml(OpenToml(U"assets/config/player.toml"));
  auto enemy = EnemyConfig::FromToml(OpenToml(U"assets/config/enemy.toml"));
  auto arena = ArenaConfig::FromToml(OpenToml(U"assets/config/arena.toml"));
  auto title = TitleConfig::FromToml(OpenToml(U"assets/config/title.toml"));
  auto input = InputConfig::FromToml(OpenToml(U"assets/config/input.toml"));
  auto stage = StageData::FromToml(OpenToml(U"assets/config/stage.toml"));
  auto anims = LoadAnimations();

  registry.ctx().get<PlayerConfig>() = std::move(player);
  registry.ctx().get<EnemyConfig>() = enemy;
  registry.ctx().get<ArenaConfig>() = arena;
  registry.ctx().get<TitleConfig>() = title;
  registry.ctx().get<InputConfig>() = input;
  registry.ctx().get<StageData>() = std::move(stage);
  registry.ctx().get<AnimationDataRegistry>() = std::move(anims);
}

}  // namespace

namespace DebugOnly {

void RunTestsIfRequested() {
  size_t envLen = 0;
  if (getenv_s(&envLen, nullptr, 0, "ASH2_RUN_TESTS") != 0) {
    throw Error{U"環境変数 ASH2_RUN_TESTS を読めない"};
  }
  if (envLen == 0) {
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
