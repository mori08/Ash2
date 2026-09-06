#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <functional>

#include "Component/PlayerMotion.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Config/PlayerConfig.hpp"
#include "ReactionLevel.hpp"

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
  /// 解放時のフェードアウト時間（秒）。0 以下なら即座に破棄する
  double fadeSec = 0.0;
  /// 珠を描画するか（false: 見た目を別の光エンティティに分離する近接攻撃用）
  bool drawOrb = true;
};

/// @brief 見た目だけを担う光エンティティ群の生成仕様をまとめた構造体
struct LightSpec {
  /// 生成する光の数
  int32 count = 1;
  /// 光の半径（CircleDrawable の表示半径）
  double radius = 0.0;
  /// 解放時のフェードアウト時間（秒）。0 以下なら即座に破棄する
  double fadeSec = 0.0;
};

/// @brief クリップが変化していれば差し替え、再生位置をリセットする
void SetClip(SpriteAnimation& anim, const String& clip);

/// @brief 横方向の速度を止める
void StopHorizontalMovement(entt::registry& registry, entt::entity entity);

/// @brief ヒットボックスを攻撃判定から解放し、消滅演出へ引き渡す
///
/// `Hierarchy::Detach` で親から切り離し、`Attack`/`Collider` を外して
/// 当たり判定から除外したうえで `FadeOut` を付与する。`fadeSec` が 0
/// 以下の場合はフェードを挟まず即座に破棄する。
void ReleaseAttackHitbox(
    entt::registry& registry, entt::entity hitboxEntity, double fadeSec
);

/// @brief 攻撃判定の発生区間に応じてヒットボックスを生成・更新・破棄する
///
/// 未生成の判定は所有者の子に `AttackHitboxOrb` が無いことで判定する。
/// @param timeline 攻撃のタイムライン（active 区間の判定に使用）
/// @param spec
/// 生成時に確定させる半径・ダメージ・リアクション・ヒットストップ時間
/// @param offsetFn 攻撃フレーム内の進行度から珠のオフセットを算出する関数
void UpdateAttackHitbox(
    entt::registry& registry, entt::entity owner, double elapsed,
    const MotionTimeline& timeline, const HitboxSpec& spec,
    const std::function<Vec3(double)>& offsetFn
);

/// @brief 攻撃判定の発生区間に応じて見た目専用の光エンティティ群を
/// 生成・更新・破棄する
///
/// 未生成の光は所有者の子に `AttackLightOrb` が無いことで判定する。
/// @param timeline 攻撃のタイムライン（active 区間の判定に使用）
/// @param spec 生成時に確定させる光の数・半径・フェード時間
/// @param offsetFn 攻撃フレーム内の進行度と光のインデックス（0始まり）から
/// オフセットを算出する関数
void UpdateAttackLights(
    entt::registry& registry, entt::entity owner, double elapsed,
    const MotionTimeline& timeline, const LightSpec& spec,
    const std::function<Vec3(double, int32)>& offsetFn
);

/// @brief 所有者の子のうち `AttackOrb` を持つものをすべて解放する
///
/// 着地・被弾による中断経路の後始末に使う（正常終了時の解放は
/// `UpdateAttackHitbox`/`UpdateAttackLights` が担い、ここは通らない）。
/// 走査しながら解放すると `Hierarchy::Detach`
/// が連結を切ってしまうため、対象を先に集めてから `ReleaseAttackHitbox`
/// を呼ぶ。所有者が子を持たない場合は何もしない。
void ReleaseAttackOrbs(
    entt::registry& registry, entt::entity owner, double fadeSec
);

}  // namespace PlayerMotion
