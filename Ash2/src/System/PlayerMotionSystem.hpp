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

/// @brief Melee1 状態の更新（横移動停止・タイマー減算・ヒットボックス管理・
/// コンボ予約判定）
/// @return 遷移先がある場合はその Motion、なければ std::nullopt
[[nodiscard]] std::optional<Motion> Tick(Melee1& state,
                                         entt::registry& registry,
                                         entt::entity entity,
                                         const FrameData& frameData);

/// @brief Melee2 状態の更新（横移動停止・タイマー減算・ヒットボックス管理・
/// コンボ予約判定）
/// @return 遷移先がある場合はその Motion、なければ std::nullopt
[[nodiscard]] std::optional<Motion> Tick(Melee2& state,
                                         entt::registry& registry,
                                         entt::entity entity,
                                         const FrameData& frameData);

/// @brief Melee3 状態の更新（横移動停止・タイマー減算・ヒットボックス管理、
/// 締め技のためコンボ継続なし）
/// @return 遷移先がある場合はその Motion、なければ std::nullopt
[[nodiscard]] std::optional<Motion> Tick(Melee3& state,
                                         entt::registry& registry,
                                         entt::entity entity,
                                         const FrameData& frameData);

/// @brief Ranged 状態の更新（横移動停止・タイマー減算）
/// @return 遷移先がある場合はその Motion、なければ std::nullopt
[[nodiscard]] std::optional<Motion> Tick(Ranged& state,
                                         entt::registry& registry,
                                         entt::entity entity,
                                         const FrameData& frameData);

}  // namespace PlayerMotion
