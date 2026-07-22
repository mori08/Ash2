#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <functional>

#include "Component/ReactionLevel.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Config/PlayerConfig.hpp"

namespace PlayerMotion {

/// @brief クリップが変化していれば差し替え、再生位置をリセットする
void SetClip(SpriteAnimation& anim, const String& clip);

/// @brief 横方向の速度を止める
void StopHorizontalMovement(entt::registry& registry, entt::entity entity);

/// @brief 攻撃判定の発生区間に応じてヒットボックスを生成・更新・破棄する
/// @param timeline 攻撃のタイムライン（active 区間の判定に使用）
/// @param radius 攻撃カプセルの半径（兼 CircleDrawable の表示半径）
/// @param damage 生成時に確定させるダメージ量
/// @param reaction 生成時に確定させる被弾側リアクションの強さ
/// @param offsetFn 攻撃フレーム内の進行度から珠のオフセットを算出する関数
void UpdateAttackHitbox(entt::registry& registry, entt::entity owner,
                        double elapsed, const MotionTimeline& timeline,
                        double radius, int32 damage, ReactionLevel reaction,
                        entt::entity& hitboxEntity,
                        const std::function<Vec3(double)>& offsetFn);

}  // namespace PlayerMotion
