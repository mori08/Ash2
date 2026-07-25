#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <functional>

#include "Component/ReactionLevel.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Config/PlayerConfig.hpp"

namespace PlayerMotion {

/// @brief 攻撃判定エンティティの生成に使う仕様をまとめた構造体
// UpdateAttackHitbox の引数で double の radius と int32 の damage が隣接すると
// 呼び出し側で取り違えやすいため、1つの構造体にまとめて渡す
// （bugprone-easily-swappable-parameters 対策）。
struct HitboxSpec {
  /// 攻撃カプセルの半径（兼 CircleDrawable の表示半径）
  double radius = 0.0;
  /// 与えるダメージ量
  int32 damage = 0;
  /// 被弾側リアクションの強さ
  ReactionLevel reaction = ReactionLevel::None;
  /// ヒット成立時に攻撃側・被弾側へ付与するヒットストップ時間（秒）
  double hitstopSec = 0.0;
};

/// @brief クリップが変化していれば差し替え、再生位置をリセットする
void SetClip(SpriteAnimation& anim, const String& clip);

/// @brief 横方向の速度を止める
void StopHorizontalMovement(entt::registry& registry, entt::entity entity);

/// @brief 攻撃判定の発生区間に応じてヒットボックスを生成・更新・破棄する
/// @param timeline 攻撃のタイムライン（active 区間の判定に使用）
/// @param spec
/// 生成時に確定させる半径・ダメージ・リアクション・ヒットストップ時間
/// @param offsetFn 攻撃フレーム内の進行度から珠のオフセットを算出する関数
void UpdateAttackHitbox(entt::registry& registry, entt::entity owner,
                        double elapsed, const MotionTimeline& timeline,
                        const HitboxSpec& spec, entt::entity& hitboxEntity,
                        const std::function<Vec3(double)>& offsetFn);

}  // namespace PlayerMotion
