#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/EnemyMotion.hpp"

struct FrameData;

namespace EnemyMotion {

/// @brief Idle 状態の更新（索敵範囲内かつ接地中なら Chase へ遷移する）
/// @return 遷移先がある場合はその状態、なければ none
[[nodiscard]] Optional<Variant> Tick(
    Idle& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Chase 状態の更新（プレイヤー方向へ moveSpeed
/// で接近しながら向きを追従させる。飛びつき開始距離まで近づいたら Windup
/// へ遷移する）
/// @return 遷移先がある場合はその状態、なければ none
[[nodiscard]] Optional<Variant> Tick(
    Chase& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Windup 状態の更新（残り時間減算。満了時はプレイヤー方向へ
/// Velocity と Attack を設定し Leap へ遷移する）
/// @return 遷移先がある場合はその状態、なければ none
[[nodiscard]] Optional<Variant> Tick(
    Windup& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Leap 状態の更新（接地かつ Velocity.h <= 0 で満了。Attack
/// を外し Landing へ遷移する）
/// @return 遷移先がある場合はその状態、なければ none
[[nodiscard]] Optional<Variant> Tick(
    Leap& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Landing 状態の更新（残り時間減算。満了で Idle へ遷移する）
/// @return 遷移先がある場合はその状態、なければ none
[[nodiscard]] Optional<Variant> Tick(
    Landing& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Stagger 状態の更新（残り時間減算。満了で Idle へ遷移する）
/// @return 遷移先がある場合はその状態、なければ none
[[nodiscard]] Optional<Variant> Tick(
    Stagger& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Repel 状態の更新（残り時間減算。満了時は Velocity.w を 0
/// に戻し Idle へ遷移する）
/// @return 遷移先がある場合はその状態、なければ none
[[nodiscard]] Optional<Variant> Tick(
    Repel& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Knockback 状態の更新（残り時間減算・接地中は Velocity.w を 0
/// に固定。放物線自体は MovementSystem/GravitySystem に委ねる。満了時は
/// Idle へ遷移する）
/// @return 遷移先がある場合はその状態、なければ none
[[nodiscard]] Optional<Variant> Tick(
    Knockback& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Defeated 状態の更新（DrawColor::color.a
/// を残り時間比でフェードアウトさせながら残り時間を減算する。満了後の破棄は
/// EnemySystem が行う）
/// @return 常に none
[[nodiscard]] Optional<Variant> Tick(
    Defeated& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

}  // namespace EnemyMotion
