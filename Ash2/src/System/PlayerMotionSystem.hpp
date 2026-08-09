#pragma once
#include <entt/entt.hpp>

#include "Component/Motion.hpp"
#include "Component/PlayerMotion.hpp"

struct FrameData;

namespace PlayerMotion {

/// @brief Neutral 状態の更新（移動・ジャンプ・向き・クリップ決定、
/// MeleeChain/Ranged/Dash への入場判定）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    Neutral& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief MeleeChain 状態の更新（横移動停止・タイマー減算・判定/光の管理・
/// コンボ予約判定。段の設定は cfg.melee.chain[state.stage] を参照する）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    MeleeChain& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief MeleeFinisher 状態の更新（横移動停止・タイマー減算・判定/光の管理。
/// コンボ継続・キャンセルは受け付けない）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    MeleeFinisher& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Ranged 状態の更新（横移動停止・タイマー減算）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    Ranged& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Dash 状態の更新（移動・タイマー減算・無敵の付与/除去・
/// 後隙Aからのダッシュ攻撃/再ダッシュキャンセル予約・後隙B開始時の遷移。
/// state.air が true の場合は空中発動として接地で Landing へ強制遷移する）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    Dash& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief DashAttack 状態の更新（突進・軌道上のヒットボックス管理・後隙満了で
/// Neutral へ戻る。state.air が true の場合は空中発動として接地で Landing
/// へ強制遷移する）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    DashAttack& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief AirAttack 状態の更新（垂直面の軌道上のヒットボックス管理・
/// 接地で Landing へ遷移・後隙満了で Neutral へ戻る）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    AirAttack& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

/// @brief Landing 状態の更新（横移動停止・タイマー減算・満了で Neutral へ戻る）
/// @return 遷移先がある場合はその Motion、なければ none
[[nodiscard]] Optional<Motion> Tick(
    Landing& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
);

}  // namespace PlayerMotion
