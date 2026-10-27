#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Config/AnimationData.hpp"

/// @brief Siv3D 側の初期設定を行う
///
/// アセット・UI フォントの登録、シーンの設定、ウィンドウプリセットの適用を
/// まとめる。
/// @throw Error アセット・UI フォントの登録やウィンドウの変更に失敗したとき
void InitializeEngine();

/// @brief registry のコンテキストを初期化する
/// @throw Error 設定の読み込みに失敗したとき、
/// またはシナリオに init セクションがないとき
void InitializeRegistry(entt::registry& registry);

/// @brief アニメーション設定 TOML を全件読み込む
/// @throw Error TOML を開けない、またはテクスチャが未登録・読み込めないとき
[[nodiscard]] AnimationDataRegistry LoadAnimations();
