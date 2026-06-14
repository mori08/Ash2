#pragma once
#include <entt/entt.hpp>
#include <optional>

#include "Component/Motion.hpp"
#include "Component/PlayerMotion.hpp"

struct FrameData;

namespace PlayerMotion {

/// @brief Neutral 状態の更新（移動・ジャンプ・向き・クリップ決定、Melee/Ranged
/// への入場判定）
/// @return 遷移先がある場合はその Motion、なければ std::nullopt
[[nodiscard]] std::optional<Motion> Tick(Neutral& state,
                                         entt::registry& registry,
                                         entt::entity entity,
                                         const FrameData& frameData);

/// @brief Melee 状態の更新（横移動停止・タイマー減算・ヒットボックス破棄）
/// @return 遷移先がある場合はその Motion、なければ std::nullopt
[[nodiscard]] std::optional<Motion> Tick(Melee& state, entt::registry& registry,
                                         entt::entity entity,
                                         const FrameData& frameData);

/// @brief Ranged 状態の更新（横移動停止・タイマー減算）
/// @return 遷移先がある場合はその Motion、なければ std::nullopt
[[nodiscard]] std::optional<Motion> Tick(Ranged& state,
                                         entt::registry& registry,
                                         entt::entity entity,
                                         const FrameData& frameData);

}  // namespace PlayerMotion
