#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <expected>

#include "Config/AnimationData.hpp"

/// @brief Siv3D 側の初期設定を行う
///
/// アセット・UI フォントの登録とテクスチャフィルタの設定をまとめる。
/// @return 失敗時は失敗した登録処理のメッセージ
[[nodiscard]] std::expected<void, String> InitializeEngine();

/// @brief registry のコンテキストを初期化する
[[nodiscard]] std::expected<void, String> InitializeRegistry(
    entt::registry& registry
);

/// @brief アニメーション設定 TOML を全件読み込む
/// @return 失敗時は toml のパスを前置したメッセージ
[[nodiscard]] std::expected<AnimationDataRegistry, String> LoadAnimations();
