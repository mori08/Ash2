#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <expected>

#include "Config/AnimationData.hpp"

/// @brief registry のコンテキストを初期化する
[[nodiscard]] std::expected<void, String> InitializeRegistry(
    entt::registry& registry
);

/// @brief アニメーション設定 TOML を全件読み込む
/// @return 失敗時は toml のパスを前置したメッセージ
[[nodiscard]] std::expected<AnimationDataRegistry, String> LoadAnimations();
