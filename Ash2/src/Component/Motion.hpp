#pragma once
#include <variant>

#include "Component/PlayerMotion.hpp"

/// @brief エンティティの排他的な行動状態（種別ごとの状態型の共有variant）
using Motion = std::variant<PlayerMotion::Neutral, PlayerMotion::Melee1,
                            PlayerMotion::Melee2, PlayerMotion::Melee3,
                            PlayerMotion::Ranged>;
