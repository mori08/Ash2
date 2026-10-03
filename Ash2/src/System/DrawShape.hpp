#pragma once
#include <Siv3D.hpp>

#include "Component/Drawable.hpp"

/// @brief Drawable 1件を画面座標・色で描く（DrawSystem・HudSystem 共通）
/// @note 最近傍サンプラーの適用はしない。呼び出し側に任せる
void DrawShape(
    const Drawable& drawable, const Vec2& screenPos, const ColorF& color
);
