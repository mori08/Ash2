#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/PlayerMotion.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"

namespace PlayerMotion {

/// @brief 指定段の Melee へ移行する（攻撃クリップを先頭から再生）
/// @param stage 移行先のコンボ段インデックス
/// @param hitboxEntity 引き継ぐ攻撃判定エンティティ（通常は entt::null）
Melee MakeMelee(
    SpriteAnimation& anim, size_t stage, entt::entity hitboxEntity = entt::null
);

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
void SpawnProjectile(
    entt::registry& registry, const WorldPos& pos, bool facingRight,
    const PlayerConfig& cfg
);

}  // namespace PlayerMotion
