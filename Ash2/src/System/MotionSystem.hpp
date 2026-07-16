#pragma once
#include <entt/entt.hpp>

#include "Component/Motion.hpp"

struct FrameData;

/// @brief Motion variant の状態型が満たすべき Tick() 契約
/// （ADL で解決される Tick(state, registry, entity, frameData) が
/// Optional<Motion> を返すこと）
template <typename S>
concept MotionState =
    requires(S& s, entt::registry& r, entt::entity e, const FrameData& f) {
      { Tick(s, r, e, f) } -> std::same_as<Optional<Motion>>;
    };

/// @brief Motion の状態遷移・更新を行う共通ディスパッチャ
class MotionSystem {
 public:
  /// @brief Motion を持つ全エンティティに対し、現在の状態に応じた Tick()
  /// を呼び、遷移先が返された場合のみ Motion を差し替える
  static void Update(entt::registry& registry, const FrameData& frameData);
};
