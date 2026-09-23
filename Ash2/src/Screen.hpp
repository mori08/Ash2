#pragma once
#include <Siv3D.hpp>

#include "Component/WorldPos.hpp"

/// 床原点の画面上のオフセット（Scene::Center() からの下方向）
inline constexpr double kFloorOriginOffsetY = 150.0;

/// @brief ワールド原点の画面座標（床原点）
[[nodiscard]] inline Vec2 WorldOrigin() {
  return Scene::CenterF().movedBy(0.0, kFloorOriginOffsetY);
}

/// @brief ワールド座標を床原点込みの画面座標へ変換する
[[nodiscard]] inline Vec2 WorldToScreen(const WorldPos& pos) {
  return WorldOrigin() + pos.toScreen();
}
