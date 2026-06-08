#pragma once
#include <entt/entt.hpp>

/// @brief registry のコンテキストを初期化する
void InitializeRegistry(entt::registry& registry);

/// @brief 設定をリロードする（Debug ビルド専用）
void ReloadConfig(entt::registry& registry);
