#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <expected>

/// @brief registry のコンテキストを初期化する
[[nodiscard]] std::expected<void, String> InitializeRegistry(
    entt::registry& registry
);

/// @brief 設定をリロードする（Debug ビルド専用）
/// @note 失敗時は旧データを維持して戻る
void ReloadConfig(entt::registry& registry);
