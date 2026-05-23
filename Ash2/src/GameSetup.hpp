#pragma once
#include <entt/entt.hpp>

/// @brief registry のコンテキストを初期化する
/// @param registry 対象レジストリ
void InitializeRegistry(entt::registry& registry);

/// @brief 設定をリロードする（Debug ビルド専用）
/// @param registry 対象レジストリ
void ReloadConfig(entt::registry& registry);
