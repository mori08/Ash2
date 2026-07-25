#include <Siv3D.hpp>

#include "System/MotionSystem.hpp"

#include "Component/Hitstop.hpp"
#include "Debug.hpp"
#include "Phase/FrameData.hpp"
#include "System/EnemyMotionSystem.hpp"
#include "System/PlayerMotionSystem.hpp"

#ifdef _DEBUG
namespace {
String MotionName(const Motion& m) {
  return std::visit(
      [](const auto& s) -> String {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, PlayerMotion::Neutral>)
          return U"Neutral";
        else if constexpr (std::is_same_v<T, PlayerMotion::Melee>)
          return U"Melee" + Format(s.stage + 1);
        else if constexpr (std::is_same_v<T, PlayerMotion::Ranged>)
          return U"Ranged";
        else if constexpr (std::is_same_v<T, PlayerMotion::Dash>)
          return s.air ? U"AirDash" : U"Dash";
        else if constexpr (std::is_same_v<T, PlayerMotion::DashAttack>)
          return s.air ? U"AirDashAttack" : U"DashAttack";
        else if constexpr (std::is_same_v<T, PlayerMotion::AirAttack>)
          return U"AirAttack";
        else if constexpr (std::is_same_v<T, PlayerMotion::Landing>)
          return U"Landing";
        else if constexpr (std::is_same_v<T, EnemyMotion::Idle>)
          return U"EnemyIdle";
        else if constexpr (std::is_same_v<T, EnemyMotion::Stagger>)
          return U"EnemyStagger";
        else if constexpr (std::is_same_v<T, EnemyMotion::Repel>)
          return U"EnemyRepel";
        else if constexpr (std::is_same_v<T, EnemyMotion::Knockback>)
          return U"EnemyKnockback";
        else if constexpr (std::is_same_v<T, EnemyMotion::Defeated>)
          return U"EnemyDefeated";
      },
      m);
}
}  // namespace
#endif

void MotionSystem::Update(entt::registry& registry,
                          const FrameData& frameData) {
  // ヒットストップ中も Tick は呼ぶ（dt = 0 で時間だけ凍結する）。
  // 除外すると停止中の入力が Tick に届かず、コンボ予約を取りこぼす
  FrameData frozen = frameData;
  frozen.dt = 0.0;

  auto view = registry.view<Motion>();
  for (const auto entity : view) {
    auto& motion = view.get<Motion>(entity);
    const FrameData& fd = registry.all_of<Hitstop>(entity) ? frozen : frameData;
    auto next = std::visit(
        [&](MotionState auto& state) {
          return Tick(state, registry, entity, fd);
        },
        motion);
    if (next.has_value()) {
      APP_LOG(U"[Motion] " + MotionName(motion) + U" → " + MotionName(*next));
      registry.replace<Motion>(entity, *next);
    }
  }
}
