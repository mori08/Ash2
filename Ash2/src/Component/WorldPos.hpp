#pragma once
#include <Siv3D.hpp>

/// @brief ワールド座標
/// @note 描画（Drawable の DrawAnchor）・当たり判定（Collider のオフセット）の
///       共通基準点。基準点が「中心」か「接地点」かはエンティティごとに異なり、
///       Drawable/Collider 側をその意味づけに合わせて設定する。
struct WorldPos {
  /// 横位置
  double w = 0.0;
  /// 高さ（地面 = 0、上方向が正）
  double h = 0.0;
  /// 奥行き（大きいほど奥 = 画面上方）
  double d = 0.0;

  /// @brief ワールド座標を画面座標に変換する
  /// @return 画面座標（右方向・下方向が正）
  [[nodiscard]] Vec2 toScreen() const { return {w, -(d + h)}; }

  /// @brief 地面上にいるか
  /// @return 高さが 0 以下なら true
  [[nodiscard]] bool isOnGround() const { return h <= 0.0; }
};
