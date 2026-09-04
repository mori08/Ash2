#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/Collider.hpp"
#include "Component/WorldPos.hpp"

/// @brief Collider のデバッグ描画（呼び出しは _DEBUG ビルドに限る）
// DrawSystem・HudSystem とは別クラスにし、本番の描画パスへ _DEBUG
// 分岐を持ち込まない。DrawCapsule・DrawGroundLine は、ロック判定の可視化が
// 拡大後のコライダーを描く際にも個別に呼べるよう公開する。
class DebugDrawSystem {
 public:
  /// @brief Attack/Hp の有無で3種に分け、色違いでコライダーの輪郭と
  /// 接地線を描く
  static void DrawColliders(const entt::registry& registry);

  /// @brief カプセルの輪郭（半円2つ＋側面2本）を描く
  static void DrawCapsule(
      const WorldPos& origin, const Collider& capsule, const ColorF& color
  );

  /// @brief カプセルの中点から接地点（h = 0）へ点線を引く
  // 疑似3Dでは h と d が画面上で潰れるため、カプセルが宙に浮いているかどうかを
  // この線の長さで示す
  // DrawCapsule には含めない。ロック判定の可視化は拡大後のカプセルを実寸の
  // カプセルへ重ねて描くが、線分の中点は拡大で変わらないため、含めると
  // 接地線が同じ位置に二重に出てしまう
  static void DrawGroundLine(
      const WorldPos& origin, const Collider& capsule, const ColorF& color
  );
};
