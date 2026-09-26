#include "DebugOnly.hpp"

#ifdef _DEBUG

#include <Siv3D.hpp>

#include <cstdlib>

#include "Asset.hpp"
#include "Config/ArenaConfig.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "CrashHandler.hpp"
#include "Debug.hpp"
#include "GameSetup.hpp"
#include "System/DebugDrawSystem.hpp"

#define CATCH_CONFIG_RUNNER
#include <ThirdParty/Catch2/catch.hpp>

namespace {

// Siv3D 予約キー（割り当て不可）:
//   F1 = ライセンス表示 / F12・PrintScreen = スクリーンショット
constexpr Input kConfigReloadKey = KeyF5;
constexpr Input kColliderDrawKey = KeyF2;
constexpr Input kEnemySpawnKey = Key4;

/// Collider のデバッグ描画を表示中か（既定は非表示）
bool colliderDrawEnabled = false;

/// @brief 設定を再読込する
/// @note 失敗時は旧データを維持して戻る
void ReloadConfig(entt::registry& registry) {
  auto playerToml = OpenToml(U"assets/config/player.toml");
  if (!playerToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + playerToml.error());
    return;
  }
  auto player = PlayerConfig::FromToml(*playerToml);
  if (!player) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + player.error());
    return;
  }

  auto enemyToml = OpenToml(U"assets/config/enemy.toml");
  if (!enemyToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + enemyToml.error());
    return;
  }
  auto enemy = EnemyConfig::FromToml(*enemyToml);
  if (!enemy) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + enemy.error());
    return;
  }

  auto arenaToml = OpenToml(U"assets/config/arena.toml");
  if (!arenaToml) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + arenaToml.error());
    return;
  }
  auto arena = ArenaConfig::FromToml(*arenaToml);
  if (!arena) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + arena.error());
    return;
  }

  auto anims = LoadAnimations();
  if (!anims) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + anims.error());
    return;
  }

  registry.ctx().get<PlayerConfig>() = *std::move(player);
  registry.ctx().get<EnemyConfig>() = *std::move(enemy);
  registry.ctx().get<ArenaConfig>() = *std::move(arena);
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
  ExitImmediately(result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
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

bool IsConfigReloadRequested() { return kConfigReloadKey.down(); }

void DrawColliders(const entt::registry& registry) {
  if (kColliderDrawKey.down()) {
    colliderDrawEnabled = !colliderDrawEnabled;
  }
  if (colliderDrawEnabled) {
    DebugDrawSystem::DrawColliders(registry);
    DebugDrawSystem::DrawBoundary(registry);
  }
}

bool IsEnemySpawnRequested() { return kEnemySpawnKey.down(); }

}  // namespace DebugOnly

#endif
