#pragma once
#include <Siv3D.hpp>

/// @brief エンティティの陣営
///
/// 攻撃判定・被弾判定を持つエンティティに付与する。同じ値どうしのヒットは
/// HitSystem が捨てる（自己ヒット・同士討ちの防止）。
enum class Team : uint8 {
  /// プレイヤーとその攻撃
  Player,
  /// 敵とその攻撃
  Enemy,
};
