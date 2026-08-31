#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <functional>
#include <variant>

#include "Component/PlayerMotion.hpp"
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
/// `Hierarchy::Detach` で親から切り離し、`Attack` と `Collider` を外して
/// 当たり判定から除外したうえで `FadeOut` を付与する。`fadeSec` が 0 以下の
/// 場合はフェードを挟まず即座に破棄する。
/// @return entt::null（呼び出し側の hitboxEntity 変数への代入に使う）
entt::entity ReleaseAttackHitbox(
    entt::registry& registry, entt::entity hitboxEntity, double fadeSec
);

/// @brief 攻撃判定の発生区間に応じてヒットボックスを生成・更新・破棄する
/// @param timeline 攻撃のタイムライン（active 区間の判定に使用）
/// @param spec
/// 生成時に確定させる半径・ダメージ・リアクション・ヒットストップ時間
/// @param offsetFn 攻撃フレーム内の進行度から珠のオフセットを算出する関数
void UpdateAttackHitbox(
    entt::registry& registry, entt::entity owner, double elapsed,
    const MotionTimeline& timeline, const HitboxSpec& spec,
    entt::entity& hitboxEntity, const std::function<Vec3(double)>& offsetFn
);

/// @brief 攻撃判定の発生区間に応じて見た目専用の光エンティティ群を
/// 生成・更新・破棄する
/// @param timeline 攻撃のタイムライン（active 区間の判定に使用）
/// @param spec 生成時に確定させる光の数・半径・フェード時間
/// @param offsetFn 攻撃フレーム内の進行度と光のインデックス（0始まり）から
/// オフセットを算出する関数
void UpdateAttackLights(
    entt::registry& registry, entt::entity owner, double elapsed,
    const MotionTimeline& timeline, const LightSpec& spec,
    Array<entt::entity>& lightEntities,
    const std::function<Vec3(double, int32)>& offsetFn
);

/// @brief 状態が hitboxEntity / lightEntities を持つ場合のみ解放する
///
/// `requires` で対象フィールドの有無を判定するため、持たない状態型を渡しても
/// 何もしない。
template <typename S>
void ReleaseMotionEntities(
    entt::registry& registry, const S& state, double fadeSec
) {
  if constexpr (requires { state.hitboxEntity; }) {
    if (state.hitboxEntity != entt::null) {
      ReleaseAttackHitbox(registry, state.hitboxEntity, fadeSec);
    }
  }
  if constexpr (requires { state.lightEntities; }) {
    for (const auto light : state.lightEntities) {
      ReleaseAttackHitbox(registry, light, fadeSec);
    }
  }
}

/// @brief 現在の Motion が持つ攻撃判定・光エンティティを解放する
///
/// 外部要因による強制遷移（被弾）で上書きされる前の後始末に使う
/// （ARCHITECTURE.md の「例外：外部要因による強制遷移」を参照）。
void ReleaseMotionEntities(
    entt::registry& registry, const Variant& motion, double fadeSec
);

}  // namespace PlayerMotion
