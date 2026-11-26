#include "System/RippleSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "Component/Ripple.hpp"

namespace {

/// @brief 横半径から楕円の半径を作る
[[nodiscard]] Vec2 EllipseRadius(double radiusX, double aspect) {
  return Vec2{radiusX, radiusX * aspect};
}

}  // namespace

void RippleSystem::Spawn(
    entt::registry& registry, const Array<BubblePop>& pops,
    const RippleConfig& config
) {
  for (const auto& pop : pops) {
    // 水面で farScale、下端で 1
    const double scale = Math::Lerp(config.farScale, 1.0, pop.depthRate);
    const double startRadius = config.startRadius * scale;

    const auto entity = registry.create();
    registry.emplace<Ripple>(
        entity,
        Ripple{
            .elapsed = 0.0,
            .life = config.life,
            .startRadius = startRadius,
            .endRadius = config.endRadius * scale,
            .aspect = config.aspect
        }
    );
    // 泡と同じ遠近の規則で、手前ほど後に描く
    registry.emplace<DrawPos>(
        entity,
        DrawPos{
            .pos = pop.pos, .layer = DrawLayer::kBackground, .depth = -pop.pos.y
        }
    );
    registry.emplace<Drawable>(
        entity,
        EllipseFrameDrawable{
            .radius = EllipseRadius(startRadius, config.aspect),
            .thickness = config.thickness
        }
    );
    registry.emplace<DrawColor>(
        entity, DrawColor{.color = ColorF{config.color, 1.0}}
    );
  }
}

void RippleSystem::Update(entt::registry& registry, double dt) {
  Array<entt::entity> expired;
  for (const auto [entity, ripple, drawable, drawColor] :
       registry.view<Ripple, Drawable, DrawColor>().each()) {
    ripple.elapsed += dt;
    if (ripple.elapsed >= ripple.life) {
      expired.push_back(entity);
      continue;
    }

    const double progress = ripple.elapsed / ripple.life;
    const double radiusX = Math::Lerp(
        ripple.startRadius, ripple.endRadius, EaseOutCubic(progress)
    );
    std::get<EllipseFrameDrawable>(drawable).radius =
        EllipseRadius(radiusX, ripple.aspect);
    drawColor.color.a = 1.0 - progress;
  }
  for (const auto entity : expired) {
    registry.destroy(entity);
  }
}
