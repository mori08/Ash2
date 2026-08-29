#include <Siv3D.hpp>

#include "FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

Optional<Variant> Tick(
    Landing& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  StopHorizontalMovement(registry, entity);

  auto& anim = registry.get<SpriteAnimation>(entity);
  SetClip(anim, U"landing");

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    return Neutral{};
  }

  return none;
}

}  // namespace PlayerMotion
