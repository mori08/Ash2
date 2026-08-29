#include <Siv3D.hpp>

#include "System/MotionSystem.hpp"

#include "Component/Hitstop.hpp"
#include "FrameData.hpp"
#include "System/EnemyMotionSystem.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace {
/// @brief variant M を持つエンティティの Tick を回す
template <typename M>
void UpdateMotion(
    entt::registry& registry, const FrameData& frameData,
    const FrameData& frozen
) {
  auto view = registry.view<M>();
  for (auto&& [entity, motion] : view.each()) {
    const FrameData& fd = registry.all_of<Hitstop>(entity) ? frozen : frameData;
    auto next = std::visit(
        [&](MotionState<M> auto& state) {
          return Tick(state, registry, entity, fd);
        },
        motion
    );
    if (next.has_value()) {
      registry.replace<M>(entity, *next);
    }
  }
}

/// @brief Ms の並び順どおりに各 variant を走査する
template <typename... Ms>
void UpdateMotions(
    entt::registry& registry, const FrameData& frameData,
    const FrameData& frozen
) {
  (UpdateMotion<Ms>(registry, frameData, frozen), ...);
}
}  // namespace

void MotionSystem::Update(
    entt::registry& registry, const FrameData& frameData
) {
  // ヒットストップ中も Tick は呼ぶ（dt = 0 で時間だけ凍結する）。
  // 除外すると停止中の入力が Tick に届かず、コンボ予約を取りこぼす
  FrameData frozen = frameData;
  frozen.dt = 0.0;

  UpdateMotions<PlayerMotion::Variant, EnemyMotion::Variant>(
      registry, frameData, frozen
  );
}
