#pragma once
#include <entt/entt.hpp>

#include "Component/EnemyMotion.hpp"
#include "Component/PlayerMotion.hpp"

struct FrameData;

/// @brief Motion variant M の状態型 S が満たすべき Tick() 契約
/// （ADL で解決される Tick(state, registry, entity, frameData) が
/// Optional<M> を返すこと）
template <typename S, typename M>
concept MotionState =
    requires(S& s, entt::registry& r, entt::entity e, const FrameData& f) {
      { Tick(s, r, e, f) } -> std::same_as<Optional<M>>;
    };

/// @brief Motion の状態遷移・更新を行う共通ディスパッチャ
class MotionSystem {
 public:
  /// @brief PlayerMotion::Variant / EnemyMotion::Variant を持つ全エンティティ
  /// に対し、現在の状態に応じた Tick() を呼び、遷移先が返された場合のみ
  /// 差し替える
  static void Update(entt::registry& registry, const FrameData& frameData);
};
