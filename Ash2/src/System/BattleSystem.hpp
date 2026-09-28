#pragma once
#include <entt/entt.hpp>

struct FrameData;

/// @brief 戦闘中の毎フレーム更新列と、フェーズ離脱時の後始末をまとめるシステム
class BattleSystem {
 public:
  /// @brief `HitstopSystem` から `AnimationSystem` までを固定順で呼ぶ
  /// @note 内部の呼び出し順は REFERENCE.md の「呼び出し順の制約」に従う。
  ///       システムを増減・並び替えるときはこの関数を直す
  static void Update(entt::registry& registry, const FrameData& frameData);

  /// @brief 独立エンティティ（`Projectile`・`FadeOut`）をまとめて破棄する
  static void Cleanup(entt::registry& registry);

  /// @brief 決着後の演出だけを進め、戦闘は進めない
  /// @note 内部の呼び出し順は Update と同じ相対順（Hitstop → FadeOut →
  ///       Animation）を守る
  static void UpdateAftermath(entt::registry& registry, double dt);
};
