#pragma once
#include <variant>

#include "Component/EnemyMotion.hpp"
#include "Component/PlayerMotion.hpp"

/// @brief エンティティの排他的な行動状態（種別ごとの状態型の共有variant）
using Motion = std::variant<
    PlayerMotion::Neutral, PlayerMotion::Melee, PlayerMotion::Ranged,
    PlayerMotion::Dash, PlayerMotion::DashAttack, PlayerMotion::AirAttack,
    PlayerMotion::Landing, EnemyMotion::Idle, EnemyMotion::Stagger,
    EnemyMotion::Repel, EnemyMotion::Knockback, EnemyMotion::Defeated>;
