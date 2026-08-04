#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/EnemyMotion.hpp"
#include "Component/Motion.hpp"

struct FrameData;

namespace EnemyMotion {

/// @brief Idle 状態の更新（無反応、何もしない）
/// @return 常に none
[[nodiscard]] Optional<Motion> Tick(
    Idle& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Stagger 状態の更新（RectDrawable
/// を縦縮みさせながら残り時間を減算する。満了時は EnemyConfig::size
/// で原寸に戻し Idle へ遷移する）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    Stagger& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Repel 状態の更新（残り時間減算。満了時は Velocity.w を 0
/// に戻し Idle へ遷移する）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    Repel& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Knockback 状態の更新（残り時間減算・接地中は Velocity.w を 0
/// に固定。放物線自体は MovementSystem/GravitySystem に委ねる。満了時は
/// Idle へ遷移する）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    Knockback& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Defeated 状態の更新（DrawColor::color.a
/// を残り時間比でフェードアウトさせながら残り時間を減算する。満了後の破棄は
/// EnemySystem が行う）
/// @return 常に none
[[nodiscard]] Optional<Motion> Tick(
    Defeated& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

}  // namespace EnemyMotion
