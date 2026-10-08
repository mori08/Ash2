#pragma once
#include <Siv3D.hpp>

#include "Component/Drawable.hpp"

/// @brief Drawable 1件を画面座標・色で描く（DrawSystem・HudSystem 共通）
void DrawOne(
    const Drawable& drawable, const Vec2& screenPos, const ColorF& color
);
