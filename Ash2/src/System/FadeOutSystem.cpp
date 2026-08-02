#include <Siv3D.hpp>

#include "System/FadeOutSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/FadeOut.hpp"

void FadeOutSystem::Update(entt::registry& registry, double dt) {
  // view.each() の走査中に destroy() すると走査が壊れるため、破棄対象は
  // 収集してからループの外でまとめて破棄する
  Array<entt::entity> toDestroy;

  for (auto&& [entity, fade] : registry.view<FadeOut>().each()) {
    fade.remaining -= dt;
    const double ratio =
        (fade.duration > 0.0) ? Max(0.0, fade.remaining / fade.duration) : 0.0;
    registry.get_or_emplace<DrawColor>(entity).color.a = ratio;
    if (fade.remaining <= 0.0) toDestroy.push_back(entity);
  }

  for (const auto entity : toDestroy) {
    registry.destroy(entity);
  }
}
