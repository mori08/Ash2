#pragma once

#ifdef _DEBUG
namespace AppDebug {
/// テスト実行中は true にセットされる。立っている間は APP_LOG が無効化される。
inline bool testMode = false;
/// Collider のデバッグ描画（DebugDrawSystem::DrawColliders）の表示切り替え。
/// F2（InputState::toggleDebugDraw）で反転する
inline bool drawColliders = false;
}  // namespace AppDebug
// 複数値の結合には << でなく + か Format() を使う: APP_LOG(U"n=" + Format(n))
#define APP_LOG(msg)          \
  (AppDebug::testMode         \
       ? static_cast<void>(0) \
       : static_cast<void>(Console << (msg)))
#else
#define APP_LOG(msg) static_cast<void>(0)
#endif
