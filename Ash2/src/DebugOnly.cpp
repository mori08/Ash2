#include "DebugOnly.hpp"

#ifdef _DEBUG

#include <Siv3D.hpp>

#include <cstdlib>

#include "Asset.hpp"
#include "Component/Attack.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "CrashHandler.hpp"
#include "Debug.hpp"
#include "GameSetup.hpp"

#define CATCH_CONFIG_RUNNER
#include <ThirdParty/Catch2/catch.hpp>

namespace {

// Siv3D 予約キー（割り当て不可）:
//   F1 = ライセンス表示 / F12・PrintScreen = スクリーンショット
constexpr Input kConfigReloadKey = KeyF5;
constexpr Input kStaggerKey = Key1;
constexpr Input kRepelKey = Key2;
constexpr Input kBlowKey = Key3;

// TODO(#115): 敵の攻撃モーション・AI が未実装
// デバッグキー（Key1/Key2/Key3）で敵に攻撃力を仮付与し、被弾確認を代替する。
// 値は仮値のため config 化しない
constexpr int32 kDebugAttackDamage = 10;
constexpr double kDebugAttackHitstopSec = 0.05;

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

  auto anims = LoadAnimations();
  if (!anims) {
    APP_LOG(U"ReloadConfig: 旧データを維持 / " + anims.error());
    return;
  }

  registry.ctx().get<PlayerConfig>() = *std::move(player);
  registry.ctx().get<EnemyConfig>() = *std::move(enemy);
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

void ApplyHitReactionTest(entt::registry& registry, entt::entity target) {
  if (target == entt::null) return;

  Optional<ReactionLevel> reaction;
  if (kStaggerKey.down()) reaction = ReactionLevel::Stagger;
  if (kRepelKey.down()) reaction = ReactionLevel::Repel;
  if (kBlowKey.down()) reaction = ReactionLevel::Blow;
  if (!reaction) return;

  registry.emplace<Attack>(
      target,
      Attack{
          .damage = kDebugAttackDamage,
          .hitstopSec = kDebugAttackHitstopSec,
          .reaction = *reaction
      }
  );
}

void ClearHitReactionTest(entt::registry& registry, entt::entity target) {
  if (target == entt::null) return;
  registry.remove<Attack>(target);
}

}  // namespace DebugOnly

#endif
