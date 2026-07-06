#include <Siv3D.hpp>

#include "System/MotionSystem.hpp"

#include "Component/Hitstop.hpp"
#include "Debug.hpp"
#include "Phase/FrameData.hpp"
#include "System/PlayerMotionSystem.hpp"

#ifdef _DEBUG
namespace {
String MotionName(const Motion& m) {
  return std::visit(
      [](const auto& s) -> String {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, PlayerMotion::Neutral>)
          return U"Neutral";
        else if constexpr (std::is_same_v<T, PlayerMotion::Melee1>)
          return U"Melee1";
        else if constexpr (std::is_same_v<T, PlayerMotion::Melee2>)
          return U"Melee2";
        else if constexpr (std::is_same_v<T, PlayerMotion::Melee3>)
          return U"Melee3";
        else if constexpr (std::is_same_v<T, PlayerMotion::Ranged>)
          return U"Ranged";
        else if constexpr (std::is_same_v<T, PlayerMotion::Dash>)
          return U"Dash";
        else if constexpr (std::is_same_v<T, PlayerMotion::DashAttack>)
          return U"DashAttack";
        else if constexpr (std::is_same_v<T, PlayerMotion::AirAttack>)
          return U"AirAttack";
        else if constexpr (std::is_same_v<T, PlayerMotion::AirDash>)
          return U"AirDash";
        else if constexpr (std::is_same_v<T, PlayerMotion::AirDashAttack>)
          return U"AirDashAttack";
        else if constexpr (std::is_same_v<T, PlayerMotion::Landing>)
          return U"Landing";
      },
      m);
}
}  // namespace
#endif

void MotionSystem::Update(entt::registry& registry,
                          const FrameData& frameData) {
  auto view = registry.view<Motion>(entt::exclude<Hitstop>);
  for (const auto entity : view) {
    auto& motion = view.get<Motion>(entity);
    auto next = std::visit(
        [&](auto& state) { return Tick(state, registry, entity, frameData); },
        motion);
    if (next.has_value()) {
      APP_LOG(U"[Motion] " + MotionName(motion) + U" → " + MotionName(*next));
      registry.replace<Motion>(entity, *next);
    }
  }
}
