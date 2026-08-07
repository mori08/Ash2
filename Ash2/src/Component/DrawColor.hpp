#pragma once
#include <Siv3D.hpp>

/// @brief DrawColor を持たないエンティティに使う既定色（白・不透明）
inline constexpr ColorF kDefaultDrawColor{1.0};

/// @brief このエンティティを描画するときの色
///
/// 図形では塗り色、テクスチャでは乗算色として使う。
struct DrawColor {
  ColorF color = kDefaultDrawColor;
};
