#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/PlayerMotion.hpp"
#include "Component/ReactionLevel.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Config/PlayerConfig.hpp"

namespace PlayerMotion {

/// @brief 指定段の MeleeChain へ移行する（攻撃クリップを先頭から再生）
/// @param stage 移行先のコンボ段インデックス
MeleeChain MakeMeleeChain(SpriteAnimation& anim, size_t stage);

/// @brief MeleeFinisher へ移行する（締め技クリップを先頭から再生）
MeleeFinisher MakeMeleeFinisher(SpriteAnimation& anim);

/// @brief Ranged へ移行する（スタミナ消費、遠距離攻撃クリップの設定と timer
/// の算出）
Ranged MakeRanged(
    entt::registry& registry, entt::entity entity, const PlayerConfig& cfg,
    SpriteAnimation& anim
);

/// @brief Dash へ移行する（スタミナ消費、クリップの設定）
/// @param air 空中発動か（true: 空中ダッシュ相当）
Dash MakeDash(
    entt::registry& registry, entt::entity entity, const PlayerConfig& cfg,
    SpriteAnimation& anim, bool air
);

/// @brief DashAttack へ移行する
/// @param air 空中発動か（true: 空中ダッシュ攻撃相当）
/// @param dashDir ダッシュ時の移動方向（正規化済み）
DashAttack MakeDashAttack(SpriteAnimation& anim, bool air, Vec2 dashDir);

/// @brief AirAttack へ移行する
AirAttack MakeAirAttack(SpriteAnimation& anim);

/// @brief 遠距離攻撃の弾エンティティを生成する
///
/// `owner` が `LockOn` を持ち `target` が有効なら狙点へ向かう方向で撃ち、
/// `anim.facingRight` をその方向の w 成分の符号で書き換える。ロックが
/// なければ従来どおり `anim.facingRight` の正面へ撃つ。
void SpawnProjectile(
    entt::registry& registry, entt::entity owner, const PlayerConfig& cfg,
    SpriteAnimation& anim
);

/// @brief 被弾による強制遷移（Stagger または Knockback）を生成する
///
/// 上書き前の Motion が持つ攻撃判定・光エンティティを解放し、Velocity の
/// リセットと Invincible の除去を行ったうえで、`reaction` と接地状態に応じて
/// Stagger（地上・Repel 以下）または Knockback（空中・Blow）を返す。
/// @param knockbackSign 吹き飛ばし方向の符号（攻撃側との位置関係から決める、
/// +1.0 または -1.0）
Variant MakeDamaged(
    entt::registry& registry, entt::entity entity, const PlayerConfig& cfg,
    SpriteAnimation& anim, ReactionLevel reaction, double knockbackSign
);

}  // namespace PlayerMotion
