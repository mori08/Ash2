#include <Siv3D.hpp>

#include "System/MotionSystem.hpp"

#include "Component/Hitstop.hpp"
#include "Phase/FrameData.hpp"
#include "System/EnemyMotionSystem.hpp"
#include "System/PlayerMotionSystem.hpp"

void MotionSystem::Update(
    entt::registry& registry, const FrameData& frameData
) {
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
        motion
    );
    if (next.has_value()) {
      registry.replace<Motion>(entity, *next);
    }
  }
}
