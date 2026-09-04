#pragma once
#include <Siv3D.hpp>

#include "Component/WorldPos.hpp"

/// @brief ワールド座標をカメラオフセット込みの画面座標へ変換する
[[nodiscard]] inline Vec2 WorldToScreen(const WorldPos& pos) {
  return Scene::Center() + pos.toScreen();
}
