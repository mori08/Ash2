#pragma once

#include <entt/entt.hpp>

/// @brief Debug ビルドにのみ存在する機能とそのキー判定
/// @note Release では空の inline 関数になるため、呼び出し側は `#ifdef _DEBUG`
/// で囲まず無条件に呼ぶ。キー割り当ては DebugOnly.cpp 冒頭にまとめてある
namespace DebugOnly {

#ifdef _DEBUG
/// @brief 環境変数 ASH2_RUN_TESTS が設定されていればテストを実行して終了する
void RunTestsIfRequested();

/// @brief デバッグ出力の準備を行う
void OpenDebugConsole();

/// @brief 設定リロードキーが押されていれば設定を再読込する
void UpdateConfigReload(entt::registry& registry);

/// @brief 設定リロードキーが押されたか
[[nodiscard]] bool IsConfigReloadRequested();

/// @brief 被弾リアクション確認用のデバッグキーが押されていれば target
/// に攻撃力を仮付与する
void ApplyHitReactionTest(entt::registry& registry, entt::entity target);

/// @brief ApplyHitReactionTest で仮付与した攻撃力を除去する
void ClearHitReactionTest(entt::registry& registry, entt::entity target);

/// @brief Collider 描画トグルキーが押されていれば表示を切り替え、
/// 表示中なら DebugDrawSystem::DrawColliders を呼ぶ
void DrawColliders(const entt::registry& registry);

/// @brief 敵追加のデバッグキーが押されたか
[[nodiscard]] bool IsEnemySpawnRequested();
#else
inline void RunTestsIfRequested() {}
inline void OpenDebugConsole() {}
inline void UpdateConfigReload(entt::registry&) {}
[[nodiscard]] inline bool IsConfigReloadRequested() { return false; }
inline void ApplyHitReactionTest(entt::registry&, entt::entity) {}
inline void ClearHitReactionTest(entt::registry&, entt::entity) {}
inline void DrawColliders(const entt::registry&) {}
[[nodiscard]] inline bool IsEnemySpawnRequested() { return false; }
#endif

}  // namespace DebugOnly
